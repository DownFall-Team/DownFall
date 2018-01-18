//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: Entity to managed projected lights.
//
//=============================================================================

#include "cbase.h"
#include "c_env_light_projected.h"
#include "c_env_light_projected_manager.h"

#define THINKTIME 0.1f

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_EnvLightProjectedManager, DT_EnvLightProjectedManager, CEnvLightProjectedManager )
	RecvPropInt( RECVINFO( m_nMaxShadowingLights ) ),
END_RECV_TABLE()

C_EnvLightProjectedManager *g_pEnvLightProjectedManager = NULL;

C_EnvLightProjectedManager::C_EnvLightProjectedManager( void )
{
	Assert( !g_pEnvLightProjectedManager );
	g_pEnvLightProjectedManager = this;
}

C_EnvLightProjectedManager::~C_EnvLightProjectedManager( void )
{
	g_pEnvLightProjectedManager = NULL;
}

void C_EnvLightProjectedManager::Spawn()
{
	SetNextClientThink( gpGlobals->curtime + THINKTIME );
}

void C_EnvLightProjectedManager::AddLight( C_EnvLightProjected *light )
{
	m_lights.AddToHead( light );
	ReSort();
}

void C_EnvLightProjectedManager::RemoveLight( C_EnvLightProjected *light )
{
	const int index = m_lights.Find( light );
	if ( m_lights.IsValidIndex( index ) )
	{
		m_lights.Remove( index );
	}
}

void C_EnvLightProjectedManager::ReSort()
{
	const int c = m_lights.Count();
	for ( int i = 0; i < c; i++ )
	{
		for ( int j = i + 1; j < c; j++ )
		{
			C_EnvLightProjected* &src = m_lights[i];
			C_EnvLightProjected* &dest = m_lights[j];

			if ( src->GetEnableRadius() > dest->GetEnableRadius() )
				V_swap( src, dest );
		}
	}
}

void C_EnvLightProjectedManager::ClientThink()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	CUtlVector<int> lightsInRange;
	CUtlVector<int> lightsNotInRange;

	const int c = m_lights.Count();
	for ( int i = 0; i < c; i++ )
	{
		C_EnvLightProjected* pLight = m_lights[i];
		if ( !pLight || !pLight->IsLightOn() ) continue;

		if ( ( pLight->GetLightOrigin() - pPlayer->GetAbsOrigin() ).LengthSqr() <= Square( pLight->GetEnableRadius() ) && engine->IsBoxInViewCluster( pLight->GetAbsOrigin(), pLight->GetAbsOrigin() ) )
		{
			lightsInRange.AddToTail( i );
		}
		else
		{
			lightsNotInRange.AddToTail( i );
		}
	}

	if ( lightsInRange.Count() > 0 )
	{
		CUtlVector<int> chosenOnes;
		GetBestCandidateForShadows( lightsInRange, lightsNotInRange, chosenOnes, m_nMaxShadowingLights );

		for ( int index : lightsNotInRange )
		{
			m_lights[index]->SetManagerShadowState( false );
		}

		// Turn on The Chosen One
		for ( int index : chosenOnes )
		{
			m_lights[index]->SetManagerShadowState( true );
		}
	}

	SetNextClientThink( gpGlobals->curtime + THINKTIME );
}

void C_EnvLightProjectedManager::GetBestCandidateForShadows( CUtlVector<int> &candidates, CUtlVector<int> &rejects, CUtlVector<int> &actives, int maxActive )
{
	const int c = candidates.Count();
	for ( int i = 0; i < c; i++ )
	{
		if ( actives.Count() < maxActive )
		{
			actives.AddToTail( candidates[i] );
		}
		else
		{
			rejects.AddToTail( candidates[i] );
		}
	}
}