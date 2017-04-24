#ifndef TRIGGER_GUNFIRE_H
#define TRIGGER_GUNFIRE_H

#include "cbase.h"
#include "utlvector.h"
#include "triggers.h"

//-----------------------------------------------------------------------------
// Purpose: Triggers whenever a gun is fired in it or through it.
//-----------------------------------------------------------------------------
class CTriggerGunFire : public CTriggerMultiple
{
	DECLARE_CLASS(CTriggerGunFire, CTriggerMultiple);
	DECLARE_DATADESC();

public:
	~CTriggerGunFire();

	void Spawn(void);
	void OnRestore(void);
	void RecieveGunfire(CBaseEntity* pFrom = NULL);
	void RecieveExplosion(CBaseEntity* pFrom = NULL);

private:
	COutputEvent m_eOnGunfire;
	COutputEvent m_eOnExplosion;
};

CUtlVector<CHandle<CTriggerGunFire>>& GetGunFireTriggers(void);

#endif