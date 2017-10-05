#ifndef TRIGGER_GUNFIRE_H
#define TRIGGER_GUNFIRE_H
#pragma once

#include "triggers.h"

DECLARE_AUTO_LIST( ITriggerGunFire );
//-----------------------------------------------------------------------------
// Purpose: Triggers whenever a gun is fired in it or through it.
//-----------------------------------------------------------------------------
class CTriggerGunFire : public CTriggerMultiple, public ITriggerGunFire
{
	DECLARE_CLASS(CTriggerGunFire, CTriggerMultiple);
	DECLARE_DATADESC();

public:

	void Spawn(void);
	void RecieveGunfire(CBaseEntity* pFrom = NULL);
	void RecieveExplosion(CBaseEntity* pFrom = NULL);

private:
	COutputEvent m_eOnGunfire;
	COutputEvent m_eOnExplosion;
};

#endif