#pragma once

#if 0

#include <vector>
#include <deque>
#include <string>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Utility.h"
#include "Rendering.h"
#include "iUIComponent.h"
#include "iUIComponentRenderable.h"
class TextureDX11;
class iUIControl;

// This class has no special alignment requirements
class _OLD_Image2DRenderGroup : public iUIComponentRenderable
{
private:

	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:

	// Each instance in the group is represented by one of these structs
	class Instance : public iUIComponent
	{
	public:

		std::string			code;				// Used to identify an instance
		iUIControl *		control;			// Pointer to a control that this instance is part of, if applicable

		bool				render;
		INTVECTOR2			position;
		float				zorder;
		INTVECTOR2			size;
		Rotation90Degree	rotation;

		INTVECTOR2			__prevxy;			// The previous xy position that we rendered this element at; for rendering efficiency
		float				__prevz;			// The previous  z position that we rendered this element at; for rendering efficiency
		INTVECTOR2			__prevsize;			// The previous size that we rendered this element at; for rendering efficiency
		bool				__prevrender;		// The previous value of the rendering flag; for rendering efficiency	
		Rotation90Degree	__prevrotation;		// The previous rotation of this instance; for rendering efficiency

		Instance() { position = INTVECTOR2(0, 0); zorder = 0.0f; size = INTVECTOR2(1, 1); render = false; rotation = Rotation90Degree::Rotate0; }
		Instance(INTVECTOR2 _position, float _zorder, INTVECTOR2 _size, bool _render, Rotation90Degree _rotation) 
		{ 
			// Set key properties to default values
			code = ""; control = NULL;

			// Set properties based on input paramteres
			position = _position; zorder = _zorder; size = _size; render = _render; rotation = _rotation;
			
			// Initialise the previous render position variables, for rendering efficiency later
			__prevxy = INTVECTOR2(-1, -1); __prevz = -1.0f; __prevsize = INTVECTOR2(-1, -1); 
			__prevrender = true; __prevrotation = Rotation90Degree::Rotate0;
		}

		bool HasChanged(void) { 
			return ( !(	(position.x == __prevxy.x) && (position.y == __prevxy.y) && (zorder == __prevz) && 
						(size.x == __prevsize.x) && (size.y == __prevsize.y) && (render == __prevrender) &&
						(rotation == __prevrotation) ) );
		}

		void UpdateControlFields(void) {
			// Store the current parameters in their respective control fields
			__prevxy.x = position.x; __prevxy.y = position.y; __prevz = zorder; 
			__prevsize.x = size.x; __prevsize.y = size.y; __prevrender = render;
			__prevrotation = rotation;
		}

		// Standard methods to satisfy the iUIComponent interface
		std::string GetCode(void) { return code; }
		void SetCode(std::string _code) { code = _code; }
		bool GetRenderActive(void) const { return render; }
		void SetRenderActive(bool _render) { render = _render; }

		// Shutdown method to satisfy the interface requirement
		void Shutdown(void) { }
	};
	typedef std::deque<Instance> InstanceCollection;

	// Stores a reference to a particular instance, in a way that allows retrieval via several alternative methods
	struct InstanceReference
	{
		Instance *				instance;
		_OLD_Image2DRenderGroup *	rendergroup;
		int						index;
		std::string				code;
	
		InstanceReference(void) { instance = NULL; rendergroup = NULL; index = -1; code = ""; }

		InstanceReference(Instance *_instance, _OLD_Image2DRenderGroup *_rendergroup, int _index, std::string _code) {
			instance = _instance; rendergroup = _rendergroup; index = _index; code = _code;
		}
	};

	// Methods to add/remove/change instances
	CMPINLINE InstanceCollection *			GetInstances(void) { return &m_instances; }
	CMPINLINE Instance *					GetInstanceDirect(InstanceCollection::size_type index) { return &(m_instances[index]); }
	_OLD_Image2DRenderGroup::Instance *			GetInstanceByCode(std::string code);
	_OLD_Image2DRenderGroup::InstanceReference 	GetInstanceReferenceByCode(std::string code);
	_OLD_Image2DRenderGroup::Instance *			AddInstance(INTVECTOR2 pos, float zorder, INTVECTOR2 size, bool render, Rotation90Degree rotation);
	void									RemoveInstance(Instance *instance);
	void									RemoveInstance(InstanceCollection::size_type index);
	CMPINLINE InstanceCollection::size_type	GetInstanceCount(void) { return m_instances.size(); }	
	CMPINLINE Instance *					GetInstance(InstanceCollection::size_type index) 
	{ 
		if (index >= m_instances.size()) return NULL; else return &(m_instances.at(index)); 
	}

	// The entire group can be set to render or not; this flag is checked by the render manager before it takes any action
	CMPINLINE bool						GetRenderActive(void) const		{ return m_render; }
	CMPINLINE void						SetRenderActive(bool render)		{ m_render = render; }

	// Return or set the z-order for this component group
	CMPINLINE float						GetZOrder(void) { return m_zorder; }
	CMPINLINE void						SetZOrder(float z) { m_zorder = z; }

	// Texture repeat
	CMPINLINE bool						IsTextureRepeating(void) const { return m_repeattexture; }
	CMPINLINE void						SetTextureRepeat(bool repeat) 
	{ 
		m_repeattexture = repeat; 
		m_forcefullupdate = true;
	}

	// Determines whether instances of this group will accept or ignore mouse input.  Default is false.
	CMPINLINE bool						AcceptsMouseInput(void) { return m_acceptsmouse; }
	CMPINLINE void						SetAcceptsMouseInput(bool flag) { m_acceptsmouse = flag; }

	CMPINLINE void ForceFullUpdateNextCycle(void) { m_forcefullupdate = true; }
	CMPINLINE void CancelFullUpdateNextCycle(void) { m_forcefullupdate = false; }

	CMPINLINE void ClearAllInstances(void) { if (m_instances.size() != 0) m_instances.clear(); }

	CMPINLINE int GetIndexCount() { return m_indexCount; }
	ID3D11ShaderResourceView* GetTexture();


public:

	// The data format used to hold index buffer data.  Note that DX11 (feature level 11.0) appears to support
	// UINT32 sized indices, but feature level 9.1 only appears to support UINT16.  Using the latter for now
	// to maintain compatibility; likely too major a change to handle via the localiser
	typedef UINT16 INDEXFORMAT;	

	_OLD_Image2DRenderGroup(void);
	~_OLD_Image2DRenderGroup(void);

	Result								Initialize(int screenWidth, int screenHeight, const std::string & texture, bool texture_repeat);
	Result								InitializeBuffers(void);

	void								Render(void);

	void								SetTexture(const std::string & name);
	void								SetTexture(TextureDX11 *tex);

	void								Shutdown(void);
	
private:
	
	Result								UpdateBuffers(void);
	void								RenderBuffers(void);

	void								UpdateVertexTextureMappingCoords(VertexType *v, Rotation90Degree rotation, float umax, float vmax);

	void								ShutdownBuffers(void);
	void								ReleaseAllInstances(void);


private:
	InstanceCollection					m_instances;		// All the instances within this render group
	int									m_bufferinstances;	// The number of instances currently allocated in the vertex buffer

	ID3D11Buffer *						m_vertexBuffer, *m_indexBuffer;
	int									m_vertexCount, m_indexCount;
	VertexType *						m_vertices;
	TextureDX11 *						m_Texture;
	INTVECTOR2							m_texturesize;		// Size of the texture to be applied to this render group
	XMFLOAT2							m_ftexturesize;		// Texture size, stored in floating point representation to avoid casts later
	bool								m_repeattexture;

	bool								m_forcefullupdate;

	float								m_zorder;			// The z-order of this component *group*, which determines order of rendering by the render manager

	int									m_screenWidth, m_screenHeight;
	float								m_screenHalfWidth, m_screenHalfHeight;
	float								m_screenLeft;

	bool								m_acceptsmouse;		// Flag determining whether this group of instances will accept or ignore mouse input
};


#endif

