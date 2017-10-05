#include "cbase.h"
#include "trigger_gunfire.h"

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(trigger_gunfire, CTriggerGunFire);

BEGIN_DATADESC(CTriggerGunFire)
	DEFINE_OUTPUT(m_eOnGunfire, "OnGunfire"),
	DEFINE_OUTPUT(m_eOnExplosion, "OnExplosion"),
END_DATADESC()

IMPLEMENT_AUTO_LIST( ITriggerGunFire );

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerGunFire::Spawn(void)
{
	BaseClass::Spawn();

	InitTrigger();
	
	// Stomp the touch function, because we don't want to respond to touch
	SetTouch(NULL);
}

void CTriggerGunFire::RecieveGunfire(CBaseEntity* pFrom)
{
	if (!m_bDisabled)
		m_eOnGunfire.FireOutput(pFrom, this);
}

void CTriggerGunFire::RecieveExplosion(CBaseEntity* pFrom)
{
	if (!m_bDisabled)
		m_eOnExplosion.FireOutput(pFrom, this);
}