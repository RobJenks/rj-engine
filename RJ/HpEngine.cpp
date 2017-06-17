#include "Equip.h"
#include "Hardpoint.h"
#include "math.h"
#include "FastMath.h"
#include "Utility.h"
#include "iContainsHardpoints.h"
#include "Engine.h"
#include "SpaceEmitter.h"
#include "CoreEngine.h"
#include "ParticleEngine.h"

#include "HpEngine.h"

void HpEngine::RecalculateHardpointData()
{
	// We need to simply multiply a basis vector by our orientation to get the base thrust vector
	BaseThrustVector = XMVector3Normalize(XMVector3Rotate(BASIS_VECTOR, Orientation));

	// Make sure we are within the bounds allowed by our equipped engine.  We simply call the 
	// SetX methods here which will take care of querying the mounted equipment
	SetThrust(Thrust);
	SetTargetThrust(TargetThrust);
}

// Virtual method to allow mounting of class-specific equipment by a call to the base class instance
void HpEngine::MountEquipment(Equipment *e)  
{ 
	if (!e || (e && e->GetType() == Equip::Class::Engine)) this->MountEngine((Engine*)e); 
}

void HpEngine::MountEngine(Engine *engine)
{
	// If we are tying to mount a null engine then this is essentially unmounting the equiment
	if (engine == NULL) 
	{
		// Unmount the equipment and recalculate hardpoint data
		m_equipment = NULL;		
		RecalculateHardpointData();

		// Dispose of the engine thrust emitter
		DestroyEngineThrustEmitter();

		// Eliminate the thrust contribution of this engine, which will trigger a recalc of velocity vectors
		this->SetThrust(0.0f);		
	}
	else						// We are mounting new equipment
	{
		// Dispose of any previous engine emitter
		DestroyEngineThrustEmitter();
		
		// Mount the new equipment and recalculate hardpoint data
		m_equipment = engine;
		RecalculateHardpointData();
	
		// Initialise a new engine thrust emitter based on the new equipment
		InitialiseEngineThrustEmitter();
		UpdateEngineThrustEmitter();		
	}
}

void HpEngine::DestroyEngineThrustEmitter(void)
{
	// Dispose of our associated engine thrust object
	if (m_emitter) 
	{
		// Attempt to break any attachment that already exists, but we don't care if it doesn't
		m_emitter->DetachFromParent();

		// Dispose of the emitter space object; shutdown method will remove and deallocate 
		// the underlying particle emitter
		m_emitter->Shutdown();
	}
}

/* Constructor */
HpEngine::HpEngine(void) : Hardpoint()
{
	// Set initial pointers to null
	m_equipment = NULL;
	m_emitter = NULL;

	// Perform an initial calculation of engine hardpoint data based on initial values (incl those inherited from Hardpoint)
	this->RecalculateHardpointData();

	// Engines should all start idle
	this->SetTargetThrust(0.0f);
	this->SetThrust(0.0f);
}

/* Destructor */
HpEngine::~HpEngine(void)
{
	// No objects that need to be manually deallocated
}

/* Copy constructor */
HpEngine::HpEngine(const HpEngine &H) : Hardpoint(H)
{
	// Set initial pointers to null
	m_equipment = NULL;
	m_emitter = NULL;

	this->BaseThrustVector = H.BaseThrustVector;
	this->CachedThrustVector = H.Position;

	// All copied engine hardpoints should begin inactive.  This must come last as it relies on e.g. the above base vectors
	this->SetThrust(0.0f);
	this->SetTargetThrust(0.0f);
}

/* Assignment operator */
HpEngine &HpEngine::operator =(const HpEngine &H)
{
	Hardpoint::operator=(H);

	this->BaseThrustVector = H.BaseThrustVector;
	this->CachedThrustVector = H.Position;

	// All copied engine hardpoints should begin inactive.  This must come last as it relies on e.g. the above base vectors
	this->SetThrust(0.0f);
	this->SetTargetThrust(0.0f);

	return *this;
}

void HpEngine::RunToTargetThrust(void)
{
	// If we are already at target thrust then we need to do nothing further
	if (abs(Thrust - TargetThrust) < Game::C_EPSILON) return;

	// If we are still between update intervals then do nothing this cycle
	static float timeinterval = 0.0f;
	timeinterval += Game::TimeFactor;
	if (timeinterval < Game::C_ENGINE_SIMULATION_UPDATE_INTERVAL) return;

	// Otherwise if we are below target thrust, increase as far as possible up to that level
	Engine *e = (Engine*)m_equipment; if (!e) return;
	if		 (Thrust < TargetThrust) SetThrust(min(Thrust + (e->GetAcceleration() * timeinterval), TargetThrust));
	else if  (Thrust > TargetThrust) SetThrust(max(Thrust - (e->GetAcceleration() * timeinterval), TargetThrust));
	
	// Reset the counter for the next time interval
	timeinterval = 0.0f;
}

void HpEngine::SetThrust(float f_thrust)
{
	// Make sure we have an engine equipped in this hardpoint; if not, force to zero
	if (!this->m_equipment) 
		f_thrust = 0.0f;
	else
	{
		// If we do have an engine, constrain the thrust to be in the range permitted by this equipment
		Engine *e = (Engine*)this->m_equipment;
		f_thrust = max(f_thrust, e->GetMinThrust());
		f_thrust = min(f_thrust, e->GetMaxThrust());
	}

	// Store the new value and recalculate the cached thrust vector
	Thrust = f_thrust;
	CachedThrustVector = BaseThrustVector * f_thrust;

	// Update engine thrust rendering to match the new engine state. TODO: fix orientation of emitter
	UpdateEngineThrustEmitter();
	
	// Notify parent ship that it needs to update its physics model
	this->NotifyParentOfThrustChange();
}

void HpEngine::InitialiseEngineThrustEmitter()
{
	// We can only initialise an engine thrust emitter if we have an engine, and not if we already have an emitter
	if (!m_equipment || m_emitter) return;

	// Make sure an emitter type is specified in the engine object
	Engine *e = (Engine*)m_equipment;
	if (e->EmitterClass == NullString) return;

	// Get a reference to the parent object of this hardpoint
	iObject *obj = this->GetParentObject();
	if (!obj) return;
	
	// Attempt to create a new emitter of this class
	ParticleEmitter *p = Game::Engine->GetParticleEngine()->CreateNewParticleEmitter(
		concat("Ship")(obj->GetID())("-")(this->Code)("-ThrustEmitter").str(), e->EmitterClass);
	if (p == NULL) return;

	// Bind this particle emitter to a SpaceEmitter object
	m_emitter = new SpaceEmitter();
	m_emitter->SetEmitter(p);

	// Attach the space emitter to this ship
	obj->AddChildAttachment(m_emitter, this->Position, this->Orientation);
	if (!m_emitter->HaveParentAttachment())
	{
		delete m_emitter; m_emitter = NULL;
		delete p; p = NULL;
		return;
	}
	
	// Store constant copy of relevant emitter properties, since we will be adjusting them based on engine state
	m_emit_freqlow = p->GetParticleEmissionFrequency(ParticleEmitter::Prop::MinValue);
	m_emit_freqhigh = p->GetParticleEmissionFrequency(ParticleEmitter::Prop::MaxValue);
	m_emit_freqlowrange = m_emit_freqlow * 50.0f;
	m_emit_freqhighrange = m_emit_freqhigh * 50.0f;
	m_emit_vellow = p->GetInitialParticleVelocity(ParticleEmitter::Prop::MinValue);
	m_emit_velhigh = p->GetInitialParticleVelocity(ParticleEmitter::Prop::MaxValue);

	// Start with the emitter deactivated
	p->SetExists(true);
	p->SetEmitting(false);	
}

void HpEngine::UpdateEngineThrustEmitter(void)
{
	// Ensure we still have a valid pointer to our eqiupments and emitter objects
	if (!m_emitter || !m_equipment) return;
	
	// Get a handle to the underlying particle emitter
	ParticleEmitter *p = m_emitter->GetEmitter();
	if (!p) return;

	// Determine current thrust level as a percentage of max possible; quit if possible DIV/0 error
	float maxthrust = ((Engine*)m_equipment)->GetMaxThrust();
	if (maxthrust < Game::C_EPSILON) return;
	float thrustpc = (Thrust / maxthrust);
	float invpcsq = (1.0f - thrustpc); invpcsq *= invpcsq;

	// If we are near zero thrust then cut off the emitter entirely
	if (thrustpc < Game::C_EPSILON) 
	{
		p->SetEmitting(false);
		return;
	}

	// Update variable properties based on current thrust level
	p->SetEmitting(true);
	p->SetParticleEmissionFrequency(ParticleEmitter::Prop::MinValue, m_emit_freqlow + (invpcsq * m_emit_freqlowrange));
	p->SetParticleEmissionFrequency(ParticleEmitter::Prop::MaxValue, m_emit_freqhigh + (invpcsq * m_emit_freqhighrange));
	p->SetInitialParticleVelocity(ParticleEmitter::Prop::MinValue, Float3MultiplyScalar(m_emit_vellow, thrustpc));
	p->SetInitialParticleVelocity(ParticleEmitter::Prop::MaxValue, Float3MultiplyScalar(m_emit_velhigh, thrustpc));

}

void HpEngine::NotifyParentOfThrustChange(void)
{
	// Notify parent object that our thrust vector has changed, or will change, by flagging the change to this hardpoint
	iContainsHardpoints *s = this->GetParentHPObject();
	if (s) s->HardpointChanged(this);
}

void HpEngine::SetTargetThrust(float f_thrust)
{
	// Make sure we have equipment in this hardpoint
	if (!m_equipment) { this->TargetThrust = 0.0f; return; }
	Engine *e = (Engine*)m_equipment;

	// Set thrust value, making sure to stay within the valid range of values
	this->TargetThrust = max(min(f_thrust, e->GetMaxThrust()), e->GetMinThrust());

	// Notify the parent ship that our thrust vector is changing, so that it can update the physics model	
	this->NotifyParentOfThrustChange();
}

void HpEngine::SetTargetThrustPercentage(float percentage)
{
	// Make sure we have equipment in this hardpoint
	if (!m_equipment) { this->TargetThrust = 0.0f; return; }
	Engine *e = (Engine*)m_equipment;

	// Treat differently depending on how min & max thrust are defined
	if (e->GetMinThrust() < 0 && e->GetMaxThrust() > 0)
	{
		// If minimum and maximum thrust are either side of zero then also allow -ve percentages
		if (percentage < 0)	SetTargetThrust(-percentage * e->GetMinThrust());
		else				SetTargetThrust( percentage * e->GetMaxThrust());
	}
	else
	{
		// Otherwise, simply allow a percentage 0.0-1.0 between min and max thrust
		SetTargetThrust(e->GetMinThrust() + (percentage * (e->GetMaxThrust()- e->GetMinThrust())));
	}
}

void HpEngine::IncrementTargetThrust(void)
{
	// Make sure we have equipment in this hardpoint
	if (!m_equipment) return;
	Engine *e = (Engine*)m_equipment;

	// Determine the increment to be applied as a percentage of the total possible thrust range
	float increment = ((e->GetMaxThrust() - e->GetMinThrust()) / Game::C_THRUST_INCREMENT_PC);

	// Update the target thrust by this increment
	SetTargetThrust(this->TargetThrust + increment);
}

// TODO: these should be scaled by the timefactor?  i.e. PC *per second* ?
void HpEngine::DecrementTargetThrust(void)
{
	// Make sure we have equipment in this hardpoint
	if (!m_equipment) return;
	Engine *e = (Engine*)m_equipment;

	// Determine the -ve increment to be applied as a percentage of the total possible thrust range
	float increment = ((e->GetMaxThrust() - e->GetMinThrust()) / Game::C_THRUST_INCREMENT_PC);

	// Update the target thrust by this increment
	SetTargetThrust(this->TargetThrust - increment);
}




