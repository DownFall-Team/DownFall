#include "cbase.h"
#include "trigger_gunfire.h"


CUtlVector<CHandle<CTriggerGunFire>> g_hGunFireTriggers;

CUtlVector<CHandle<CTriggerGunFire>>& GetGunFireTriggers(void)
{
	return g_hGunFireTriggers;
}

LINK_ENTITY_TO_CLASS(trigger_gunfire, CTriggerGunFire);

BEGIN_DATADESC(CTriggerGunFire)
	DEFINE_OUTPUT(m_eOnGunfire, "OnGunfire"),
	DEFINE_OUTPUT(m_eOnExplosion, "OnExplosion"),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTriggerGunFire::~CTriggerGunFire(void)
{
	g_hGunFireTriggers.FindAndRemove(this);
}

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerGunFire::Spawn(void)
{
	BaseClass::Spawn();

	InitTrigger();

	g_hGunFireTriggers.AddToTail(this);

	// Stomp the touch function, because we don't want to respond to touch
	SetTouch(NULL);
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerGunFire::OnRestore()
{
	BaseClass::OnRestore();

	g_hGunFireTriggers.AddToTail(this);
}

void CTriggerGunFire::RecieveGunfire(CBaseEntity* pFrom)
{
	m_eOnGunfire.FireOutput(pFrom, this);
}

void CTriggerGunFire::RecieveExplosion(CBaseEntity* pFrom)
{
	m_eOnExplosion.FireOutput(pFrom, this);
}
