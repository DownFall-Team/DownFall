#include "cbase.h"
#include "c_baseentity.h"
#include "c_func_water_analog.h"

#include "tier0/memdbgon.h"

CUtlVector<EHANDLE> waters;

class C_FuncMoveLinearWater : public C_BaseEntity
{
	DECLARE_CLASS( C_FuncMoveLinearWater, C_BaseEntity )
public:
	DECLARE_CLIENTCLASS()

	void Spawn() OVERRIDE
	{
		BaseClass::Spawn();
		waters.AddToTail( this );
	}

	~C_FuncMoveLinearWater()
	{
		waters.FindAndRemove( this );
	}
};
IMPLEMENT_CLIENTCLASS_DT( C_FuncMoveLinearWater, DT_WaterAnalog, CFuncMoveLinearWater )
END_RECV_TABLE()

static bool IsPointInWaterAnalog( Vector& origin )
{
	FOR_EACH_VEC( waters, i )
	{
		C_BaseEntity* water = waters[i];
		CCollisionProperty* waterProp = water->CollisionProp();
		const Vector& waterOrigin = water->GetAbsOrigin();
		if ( origin.WithinAABox( waterOrigin + waterProp->OBBMins(), waterOrigin + waterProp->OBBMaxs() ) )
			return true;
	}
	return false;
}

bool IsPointInWaterAnalog( const Vector& origin )
{
	return IsPointInWaterAnalog( const_cast< Vector& >( origin ) );
}