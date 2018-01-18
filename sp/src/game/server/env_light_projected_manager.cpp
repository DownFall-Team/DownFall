//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: Entity to project light, optionally with realtime shadows.
//
//=============================================================================

#include "cbase.h"
#include "shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CEnvLightProjectedManager : public CPointEntity
{
	DECLARE_CLASS( CEnvLightProjectedManager, CPointEntity );
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CEnvLightProjectedManager();

	// Always transmit to clients
	virtual int UpdateTransmitState();

private:
	CNetworkVar( int, m_nMaxShadowingLights );
};

LINK_ENTITY_TO_CLASS( env_light_projected_manager, CEnvLightProjectedManager );

BEGIN_DATADESC( CEnvLightProjectedManager )
	DEFINE_KEYFIELD( m_nMaxShadowingLights, FIELD_INTEGER, "shadowingLights" )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CEnvLightProjectedManager, DT_EnvLightProjectedManager )
	SendPropInt( SENDINFO( m_nMaxShadowingLights ), 32, SPROP_UNSIGNED ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEnvLightProjectedManager::CEnvLightProjectedManager( void )
{
	m_nMaxShadowingLights = 4;
}

int CEnvLightProjectedManager::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}