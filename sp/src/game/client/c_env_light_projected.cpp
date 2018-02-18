//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: Entity to project light, optionally with realtime shadows.
//
//=============================================================================

#include "cbase.h"
#include "materialsystem/itexture.h"
#include "c_env_light_projected.h"
#include "c_env_light_projected_manager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define THINKTIME 0.1f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

IMPLEMENT_CLIENTCLASS_DT( C_EnvLightProjected, DT_EnvLightProjected, CEnvLightProjected )
	RecvPropInt(		RECVINFO( m_iEnableShadowRadius ) ),
	RecvPropVector(		RECVINFO( m_vRadiusOrigin ) ),
	RecvPropEHandle(	RECVINFO( m_hTargetEntity ) ),
	RecvPropBool(		RECVINFO( m_bState ) ),
	RecvPropBool(		RECVINFO( m_bOnlyDrawIfShadowed ) ),
	RecvPropFloat(		RECVINFO( m_flLightFOV ) ),
	RecvPropFloat(		RECVINFO( m_fLinearAtten ) ),
	RecvPropFloat(		RECVINFO( m_flFilterSize ) ),
	RecvPropBool(		RECVINFO( m_bEnableShadows ) ),
	RecvPropBool(		RECVINFO( m_bLightOnlyTarget ) ),
	RecvPropBool(		RECVINFO( m_bLightWorld ) ),
	RecvPropBool(		RECVINFO( m_bCameraSpace ) ),
	RecvPropVector(		RECVINFO( m_LinearFloatLightColor ) ),
	RecvPropFloat(		RECVINFO( m_flAmbient ) ),
	RecvPropString(		RECVINFO( m_SpotlightTextureName ) ),
	RecvPropString(		RECVINFO( m_szPattern ) ),
	RecvPropInt(		RECVINFO( m_nSpotlightTextureFrame ) ),
	RecvPropFloat(		RECVINFO( m_flNearZ ) ),
	RecvPropFloat(		RECVINFO( m_flFarZ ) ),
	RecvPropInt(		RECVINFO( m_nShadowQuality ) ),
END_RECV_TABLE()

C_EnvLightProjected::C_EnvLightProjected( void )
{
	m_LightHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_szPattern[0] = 0;
}

C_EnvLightProjected::~C_EnvLightProjected( void )
{
	RemoveFromManager();
	ShutDownLightHandle();
}

void C_EnvLightProjected::ShutDownLightHandle( void )
{
	// Clear out the light
	if ( m_LightHandle != CLIENTSHADOW_INVALID_HANDLE )
	{
		g_pClientShadowMgr->DestroyFlashlight( m_LightHandle );
		m_LightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_EnvLightProjected::OnDataChanged( DataUpdateType_t updateType )
{
	// Initialising the texture
	if ( updateType == DATA_UPDATE_CREATED )
	{
		DevMsg( "C_EnvProjectedTexture::Initialising with %s\n", m_SpotlightTextureName );
		m_SpotlightTexture.Init( materials->FindTexture( m_SpotlightTextureName, TEXTURE_GROUP_OTHER, false ) );
	}

	// Texture has changed
	if ( m_SpotlightTexture.IsValid() && Q_strcmp( m_SpotlightTexture->GetName(), m_SpotlightTextureName ) )
	{
		DevMsg( "C_EnvProjectedTexture::Shutting down reference to %s, initialising %s\n", m_SpotlightTexture->GetName(), m_SpotlightTextureName );
		m_SpotlightTexture.Shutdown( true );
		m_SpotlightTexture.Init( m_SpotlightTextureName, TEXTURE_GROUP_OTHER, false );
	}

	// Think now
	SetNextClientThink( gpGlobals->curtime );

	// Update the light
	UpdateLight( true );

	BaseClass::OnDataChanged( updateType );
}

void C_EnvLightProjected::ClientThink()
{
	// If we don't have a pattern
	if ( m_szPattern[0] == 0 )
	{
		// Reset and return
		m_flPatternBrightnessModifier = 1.0f;
		return;
	}

	// Measure the pattern length
	int patternLength = Q_strlen( m_szPattern );

	// If the pattern index is out of range
	if ( m_iPatternIndex >= patternLength )
	{
		// Wrap
		m_iPatternIndex = 0;
	}

	// Get the current intensity char
	char currentIntensity = m_szPattern[m_iPatternIndex];

	// Get intensity from alpha character
	m_flPatternBrightnessModifier = ( ( currentIntensity - 'a' ) / 25.0f );

	// Increment pattern array index
	m_iPatternIndex++;

	// If the light isn't parented
	// (parented lights	always update)
	if ( GetMoveParent() == NULL )
	{
		// Force an update
		UpdateLight( true );
	}

	// Think again
	SetNextClientThink( gpGlobals->curtime + THINKTIME );
}

bool C_EnvLightProjected::IsFineWithoutShadows()
{
	return !m_bOnlyDrawIfShadowed;
}

bool C_EnvLightProjected::IsLightOn()
{
	return m_bState;
}

Vector C_EnvLightProjected::GetLightOrigin()
{
	return GetAbsOrigin() + m_vRadiusOrigin;
}

void C_EnvLightProjected::AddToManager()
{
	if ( m_bAddedToManager || m_iEnableShadowRadius <= 0 )
		return;

	C_EnvLightProjectedManager *manager = g_pEnvLightProjectedManager;

	if ( !manager )
		return;

	manager->AddLight( this );
	m_bAddedToManager = true;
}

void C_EnvLightProjected::RemoveFromManager()
{
	if ( !m_bAddedToManager )
		return;

	C_EnvLightProjectedManager *manager = g_pEnvLightProjectedManager;

	if ( !manager )
		return;

	manager->RemoveLight( this );
	m_bAddedToManager = false;
}

int C_EnvLightProjected::GetEnableRadius()
{
	return m_iEnableShadowRadius;
}

void C_EnvLightProjected::SetManagerShadowState( bool state )
{
	if ( m_bShadowsOnClient != state )
	{
		DevMsg( 2, "Light with a radius of %i getting shadows\n", m_iEnableShadowRadius );
		m_bShadowsOnClient = state;
		ShutDownLightHandle();
	}
}

static bool IsBBoxVisible( Vector vecExtentsMin, const Vector &vecExtentsMax )
{
	// Z position clamped to the min height (but must be less than the max)
	float flVisibleBBoxMinHeight = Min( vecExtentsMax.z - 1.0f, -FLT_MAX );
	vecExtentsMin.z = MAX( vecExtentsMin.z, flVisibleBBoxMinHeight );

	// Check if the bbox is in the view
	return !engine->CullBox( vecExtentsMin, vecExtentsMax );
}

void C_EnvLightProjected::UpdateLight( bool bForceUpdate )
{
	if ( !m_bState )
		return ShutDownLightHandle();

	if ( m_bEnableShadows && !m_bAddedToManager )
		AddToManager();

	if ( !m_bAddedToManager )
	{
		m_bShadowsOnClient = m_bEnableShadows;

		if ( !materials->SupportsShadowDepthTextures() )
			m_bShadowsOnClient = false;
	}

	if ( m_bOnlyDrawIfShadowed && !m_bShadowsOnClient )
		return ShutDownLightHandle();

	Vector vForward, vRight, vUp, vPos = GetAbsOrigin();
	FlashlightState_t state;

	if ( m_hTargetEntity != NULL )
	{
		if ( m_bCameraSpace )
		{
			const QAngle &angles = GetLocalAngles();

			if ( C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer() )
			{
				const QAngle playerAngles = pPlayer->GetAbsAngles();

				Vector vPlayerForward, vPlayerRight, vPlayerUp;
				AngleVectors( playerAngles, &vPlayerForward, &vPlayerRight, &vPlayerUp );

				matrix3x4_t	mRotMatrix;
				AngleMatrix( angles, mRotMatrix );

				VectorITransform( vPlayerForward, mRotMatrix, vForward );
				VectorITransform( vPlayerRight, mRotMatrix, vRight );
				VectorITransform( vPlayerUp, mRotMatrix, vUp );

				const float dist = ( m_hTargetEntity->GetAbsOrigin() - GetAbsOrigin() ).Length();
				vPos = m_hTargetEntity->GetAbsOrigin() - vForward * dist;

				VectorNormalize( vForward );
				VectorNormalize( vRight );
				VectorNormalize( vUp );
			}
		}
		else
		{
			QAngle vecAngles;
			VectorAngles( m_hTargetEntity->GetAbsOrigin() - GetAbsOrigin(), vecAngles );
			AngleVectors( vecAngles, &vForward, &vRight, &vUp );
		}
	}
	else
	{
		AngleVectors( GetAbsAngles(), &vForward, &vRight, &vUp );
	}

	state.m_fHorizontalFOVDegrees = m_flLightFOV;
	state.m_fVerticalFOVDegrees = m_flLightFOV;

	state.m_vecLightOrigin = vPos;
	BasisToQuaternion( vForward, vRight, vUp, state.m_quatOrientation );

	state.m_fLinearAtten = m_fLinearAtten;

	state.m_Color[0] = ( m_LinearFloatLightColor.x * m_flPatternBrightnessModifier );
	state.m_Color[1] = ( m_LinearFloatLightColor.y * m_flPatternBrightnessModifier );
	state.m_Color[2] = ( m_LinearFloatLightColor.z * m_flPatternBrightnessModifier );

	state.m_NearZ = m_flNearZ;
	state.m_FarZ = m_flFarZ;

	state.m_bEnableShadows = m_bShadowsOnClient;
	state.m_pSpotlightTexture = m_SpotlightTexture;
	state.m_nSpotlightTextureFrame = m_nSpotlightTextureFrame;

	state.m_flShadowFilterSize = m_flFilterSize;

	// get the half-widths of the near and far planes, 
	// based on the FOV which is in degrees. Remember that
	// on planet Valve, x is forward, y left, and z up. 
	const float tanHalfAngle = tan( m_flLightFOV * ( M_PI / 180.0f ) * 0.5f );
	const float halfWidthNear = tanHalfAngle * m_flNearZ;
	const float halfWidthFar = tanHalfAngle * m_flFarZ;
	// now we can build coordinates in local space: the near rectangle is eg 
	// (0, -halfWidthNear, -halfWidthNear), (0,  halfWidthNear, -halfWidthNear), 
	// (0,  halfWidthNear,  halfWidthNear), (0, -halfWidthNear,  halfWidthNear)

	const VectorAligned vNearRect[4] = {
		VectorAligned( m_flNearZ, -halfWidthNear, -halfWidthNear ), VectorAligned( m_flNearZ,  halfWidthNear, -halfWidthNear ),
		VectorAligned( m_flNearZ,  halfWidthNear,  halfWidthNear ), VectorAligned( m_flNearZ, -halfWidthNear,  halfWidthNear )
	};

	const VectorAligned vFarRect[4] = {
		VectorAligned( m_flFarZ, -halfWidthFar, -halfWidthFar ), VectorAligned( m_flFarZ,  halfWidthFar, -halfWidthFar ),
		VectorAligned( m_flFarZ,  halfWidthFar,  halfWidthFar ), VectorAligned( m_flFarZ, -halfWidthFar,  halfWidthFar )
	};

	matrix3x4_t matOrientation( vForward, -vRight, vUp, vPos );

	enum
	{
		kNEAR = 0,
		kFAR = 1,
	};
	VectorAligned vOutRects[2][4];

	for ( int i = 0; i < 4; ++i )
	{
		VectorTransform( vNearRect[i].Base(), matOrientation, vOutRects[0][i].Base() );
	}
	for ( int i = 0; i < 4; ++i )
	{
		VectorTransform( vFarRect[i].Base(), matOrientation, vOutRects[1][i].Base() );
	}

	// now take the min and max extents for the bbox, and see if it is visible.
	Vector mins = static_cast< Vector >( **vOutRects );
	Vector maxs = static_cast< Vector >( **vOutRects );
	for ( int i = 1; i < 8; ++i )
	{
		VectorMin( mins, *( *vOutRects + i ), mins );
		VectorMax( maxs, *( *vOutRects + i ), maxs );
	}

#if 0 //for debugging the visibility frustum we just calculated
	NDebugOverlay::Triangle( vOutRects[0][0], vOutRects[0][1], vOutRects[0][2], 255, 0, 0, 100, true, 0.0f ); //first tri
	NDebugOverlay::Triangle( vOutRects[0][2], vOutRects[0][1], vOutRects[0][0], 255, 0, 0, 100, true, 0.0f ); //make it double sided
	NDebugOverlay::Triangle( vOutRects[0][2], vOutRects[0][3], vOutRects[0][0], 255, 0, 0, 100, true, 0.0f ); //second tri
	NDebugOverlay::Triangle( vOutRects[0][0], vOutRects[0][3], vOutRects[0][2], 255, 0, 0, 100, true, 0.0f ); //make it double sided

	NDebugOverlay::Triangle( vOutRects[1][0], vOutRects[1][1], vOutRects[1][2], 0, 0, 255, 100, true, 0.0f ); //first tri
	NDebugOverlay::Triangle( vOutRects[1][2], vOutRects[1][1], vOutRects[1][0], 0, 0, 255, 100, true, 0.0f ); //make it double sided
	NDebugOverlay::Triangle( vOutRects[1][2], vOutRects[1][3], vOutRects[1][0], 0, 0, 255, 100, true, 0.0f ); //second tri
	NDebugOverlay::Triangle( vOutRects[1][0], vOutRects[1][3], vOutRects[1][2], 0, 0, 255, 100, true, 0.0f ); //make it double sided

	NDebugOverlay::Box( vec3_origin, mins, maxs, 0, 255, 0, 100, 0.0f );
#endif

	if ( !IsBBoxVisible( mins, maxs ) )
	{
		// Spotlight's extents aren't in view
		if ( m_LightHandle != CLIENTSHADOW_INVALID_HANDLE )
			ShutDownLightHandle();

		return;
	}

	if ( m_LightHandle == CLIENTSHADOW_INVALID_HANDLE )
	{
		m_LightHandle = g_pClientShadowMgr->CreateFlashlight( state );
	}
	else
	{
		if ( m_hTargetEntity != NULL || bForceUpdate )
		{
			g_pClientShadowMgr->UpdateFlashlightState( m_LightHandle, state );
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture( m_LightHandle, true );
}

void C_EnvLightProjected::Simulate( void )
{
	UpdateLight( GetMoveParent() != NULL );

	BaseClass::Simulate();
}

