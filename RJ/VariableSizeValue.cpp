#include "VariableSizeValue.h"

#include "iObject.h"
#include "ModelInstance.h"
#include "ArticulatedModelComponent.h"


// Forward-declare obejct types that this data can be applied to
template void VariableSizeValue::ApplyToObject<iObject>(iObject *) const;
template void VariableSizeValue::ApplyToObject<ModelInstance>(ModelInstance *) const;
template void VariableSizeValue::ApplyToObject<ArticulatedModelComponent>(ArticulatedModelComponent *) const;


