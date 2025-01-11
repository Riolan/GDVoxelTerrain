#ifndef MODIFY_SETTINGS_H
#define MODIFY_SETTINGS_H

#include "jar_signed_distance_field.h"
#include "bounds.h"
#include "sdf_opterations.h"

struct ModifySettings
{
    public:
        JarSignedDistanceField* sdf;
        Bounds* bounds;
        SdfOperation* operation;
};

#endif // MODIFY_SETTINGS_H
