//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: Entity to managed projected lights.
//
//=============================================================================

#include "cbase.h"
#include "c_env_light_projected_manager.h"

#define THINKTIME 0.1f

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_EnvLightProjectedManager, DT_EnvLightProjectedManager, CEnvLightProjectedManager )
END_RECV_TABLE()

C_EnvLightProjectedManager *g_pEnvLightProjectedManager = NULL;

C_EnvLightProjectedManager::C_EnvLightProjectedManager( void )
{
	Assert(!g_pEnvLightProjectedManager);
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

void C_EnvLightProjectedManager::AddLight(C_EnvLightProjected *light)
{
	m_lights.AddToHead( light );
	ReSort();
}

void C_EnvLightProjectedManager::RemoveLight(C_EnvLightProjected *light)
{
	const int index = m_lights.Find( light );
	if (index >= 0)
	{
		m_lights.Remove( index );
	}
}

void C_EnvLightProjectedManager::ReSort()
{
	int c = m_lights.Count();
	for ( int i = 0; i < c; i++ )
	{
		for ( int j = i + 1; j < c; j++ )
		{
			C_EnvLightProjected **src = &m_lights[i];
			C_EnvLightProjected **dest = &m_lights[j];

			if ( (*src)->GetEnableRadius() > (*dest)->GetEnableRadius() )
				V_swap( *src, *dest );
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

	int c = m_lights.Count();
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
		const int enableLightIndex = GetBestCandidateForShadows(&lightsInRange, &lightsNotInRange);

		for (int i = 0; i < lightsNotInRange.Count(); i++)
		{
			int index = lightsNotInRange[i];
			m_lights[index]->SetManagerShadowState( false );
		}

		// Turn on The Chosen One
		m_lights[enableLightIndex]->SetManagerShadowState( true );
	}

	SetNextClientThink( gpGlobals->curtime + THINKTIME );
}

int C_EnvLightProjectedManager::GetBestCandidateForShadows(CUtlVector<int> *candidates, CUtlVector<int> *rejects)
{
	bool found = false;
	int chosenOne = candidates->Element( 0 );
	for ( int i = 0; i < candidates->Count(); i++ )
	{
		if ( !found )
		{
			chosenOne = candidates->Element( i );
			found = true;
		}
		else
		{
			rejects->AddToTail( candidates->Element( i ) );
		}
	}

	return chosenOne;
}