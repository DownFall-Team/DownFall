//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#include "cbase.h"
#include "shareddefs.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "texture_group_names.h"
#include "tier0/icommandline.h"
#include "view_scene.h"
#include "viewrender.h"

#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_slopescaledepthbias_shadowmap( "mat_slopescaledepthbias_shadowmap", "4", FCVAR_CHEAT );
static ConVar mat_depthbias_shadowmap( "mat_depthbias_shadowmap", "0.00001", FCVAR_CHEAT );

static ConVar volumetrics_fade_range( "volumetrics_fade_range", "128.0", FCVAR_CHEAT  );
static ConVar volumetrics_enabled( "volumetrics_enabled", "1", FCVAR_ARCHIVE );

class C_EnvProjectedTexture : public C_BaseEntity
{
	DECLARE_CLASS( C_EnvProjectedTexture, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	virtual void OnDataChanged( DataUpdateType_t updateType );
	void	ShutDownLightHandle( void );

	virtual void Simulate();

	void	UpdateLight( bool bForceUpdate );

	virtual void UpdateOnRemove()
	{
		if ( m_pVolmetricMesh != NULL )
		{
			CMatRenderContextPtr pRenderContext( materials );
			pRenderContext->DestroyStaticMesh( m_pVolmetricMesh );
			m_pVolmetricMesh = NULL;
		}
		BaseClass::UpdateOnRemove();
	}

	virtual bool					IsTransparent() { return true; }
	virtual bool					IsTwoPass() { return false; }

	virtual void GetRenderBoundsWorldspace( Vector& mins, Vector& maxs )
	{
		if ( m_bEnableVolumetrics )
		{
			mins = m_vecRenderBoundsMin;
			maxs = m_vecRenderBoundsMax;
		}
		else
		{
			BaseClass::GetRenderBoundsWorldspace( mins, maxs );
		}
	}
	virtual bool ShouldDraw() { return true; }
	virtual int DrawModel( int flags );

	virtual bool ShouldReceiveProjectedTextures( int flags ) { return false; }

	void ClearVolumetricsMesh();

	C_EnvProjectedTexture();
	~C_EnvProjectedTexture();

private:

	void RebuildVolumetricMesh();
	void GetShadowViewSetup( CViewSetup &setup );

	IMesh	*m_pVolmetricMesh;
	CMaterialReference m_matVolumetricsMaterial;

	ClientShadowHandle_t m_LightHandle;

	EHANDLE	m_hTargetEntity;

	bool	m_bState;
	float	m_flLightFOV;
	bool	m_bEnableShadows;
	bool	m_bLightOnlyTarget;
	bool	m_bLightWorld;
	bool	m_bCameraSpace;
	Vector	m_LinearFloatLightColor;
	float	m_flAmbient;
	float	m_flNearZ;
	float	m_flFarZ;
	char	m_SpotlightTextureName[MAX_PATH];
	int		m_nSpotlightTextureFrame;
	int		m_nShadowQuality;

	FlashlightState_t	m_FlashlightState;
	Vector m_vecRenderBoundsMin, m_vecRenderBoundsMax;

	bool m_bEnableVolumetrics;
	bool m_bEnableVolumetricsLOD;
	float m_flVolumetricsFadeDistance;
	int m_iVolumetricsQuality;
	float m_flVolumetricsMultiplier;
	float m_flVolumetricsQualityBias;

	float m_flLastFOV;
	int m_iCurrentVolumetricsSubDiv;
};


static void volumetrics_subdiv_Callback( IConVar *var, const char *, float )
{
	for ( C_BaseEntity *pEnt = ClientEntityList().FirstBaseEntity(); pEnt != NULL;
		pEnt = ClientEntityList().NextBaseEntity( pEnt ) )
	{
		C_EnvProjectedTexture *pProjectedTexture = dynamic_cast< C_EnvProjectedTexture* >( pEnt );
		if ( pProjectedTexture != NULL )
		{
			pProjectedTexture->ClearVolumetricsMesh();
		}
	}
}
static ConVar volumetrics_subdiv( "volumetrics_subdiv", "0", 0, "", volumetrics_subdiv_Callback );

IMPLEMENT_CLIENTCLASS_DT( C_EnvProjectedTexture, DT_EnvProjectedTexture, CEnvProjectedTexture )
	RecvPropEHandle( RECVINFO( m_hTargetEntity ) ),
	RecvPropBool( RECVINFO( m_bState ) ),
	RecvPropFloat( RECVINFO( m_flLightFOV ) ),
	RecvPropBool( RECVINFO( m_bEnableShadows ) ),
	RecvPropBool( RECVINFO( m_bLightOnlyTarget ) ),
	RecvPropBool( RECVINFO( m_bLightWorld ) ),
	RecvPropBool( RECVINFO( m_bCameraSpace ) ),
	RecvPropVector( RECVINFO( m_LinearFloatLightColor ) ),
	RecvPropFloat( RECVINFO( m_flAmbient ) ),
	RecvPropString( RECVINFO( m_SpotlightTextureName ) ),
	RecvPropInt( RECVINFO( m_nSpotlightTextureFrame ) ),
	RecvPropFloat( RECVINFO( m_flNearZ ) ),
	RecvPropFloat( RECVINFO( m_flFarZ ) ),
	RecvPropInt( RECVINFO( m_nShadowQuality ) ),

	RecvPropBool(	 RECVINFO( m_bEnableVolumetrics ) ),
	RecvPropBool(	 RECVINFO( m_bEnableVolumetricsLOD ) ),
	RecvPropFloat(	 RECVINFO( m_flVolumetricsFadeDistance ) ),
	RecvPropInt(	 RECVINFO( m_iVolumetricsQuality ) ),
	RecvPropFloat(	 RECVINFO( m_flVolumetricsQualityBias ) ),
	RecvPropFloat(	 RECVINFO( m_flVolumetricsMultiplier ) ),
END_RECV_TABLE()

C_EnvProjectedTexture::C_EnvProjectedTexture( void )
	: m_bEnableVolumetrics( false )
	, m_flLastFOV( 0.0f )
	, m_iCurrentVolumetricsSubDiv( 1 )
	, m_flVolumetricsQualityBias( 1.0f )
{
	m_LightHandle = CLIENTSHADOW_INVALID_HANDLE;
}

C_EnvProjectedTexture::~C_EnvProjectedTexture( void )
{
	Assert( m_pVolmetricMesh == NULL );

	ShutDownLightHandle();
}

void C_EnvProjectedTexture::ShutDownLightHandle( void )
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
void C_EnvProjectedTexture::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( m_flLastFOV != m_flLightFOV )
	{
		m_flLastFOV = m_flLightFOV;
		ClearVolumetricsMesh();
	}
	UpdateLight( true );
}

static bool IsBBoxVisible( Vector vecExtentsMin, const Vector &vecExtentsMax )
{
	// Z position clamped to the min height (but must be less than the max)
	float flVisibleBBoxMinHeight = Min( vecExtentsMax.z - 1.0f, -FLT_MAX );
	vecExtentsMin.z = MAX( vecExtentsMin.z, flVisibleBBoxMinHeight );

	// Check if the bbox is in the view
	return !engine->CullBox( vecExtentsMin, vecExtentsMax );
}

void C_EnvProjectedTexture::UpdateLight( bool bForceUpdate )
{
	if ( m_bState == false )
	{
		if ( m_LightHandle != CLIENTSHADOW_INVALID_HANDLE )
		{
			ShutDownLightHandle();
		}

		ClearVolumetricsMesh();
		return;
	}

	Vector vForward, vRight, vUp, vPos = GetAbsOrigin();
	if ( m_hTargetEntity != NULL )
	{
		if ( m_bCameraSpace )
		{
			const QAngle &angles = GetLocalAngles();

			C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
			if ( pPlayer )
			{
				const QAngle playerAngles = pPlayer->GetAbsAngles();

				Vector vPlayerForward, vPlayerRight, vPlayerUp;
				AngleVectors( playerAngles, &vPlayerForward, &vPlayerRight, &vPlayerUp );

				matrix3x4_t	mRotMatrix;
				AngleMatrix( angles, mRotMatrix );

				VectorITransform( vPlayerForward, mRotMatrix, vForward );
				VectorITransform( vPlayerRight, mRotMatrix, vRight );
				VectorITransform( vPlayerUp, mRotMatrix, vUp );

				float dist = ( m_hTargetEntity->GetAbsOrigin() - GetAbsOrigin() ).Length();
				vPos = m_hTargetEntity->GetAbsOrigin() - vForward*dist;

				VectorNormalize( vForward );
				VectorNormalize( vRight );
				VectorNormalize( vUp );
			}
		}
		else
		{
			QAngle vecAngles;
			if ( m_hTargetEntity == NULL )
			{
				vecAngles = GetAbsAngles();
			}
			else
			{
				Vector vecToTarget = m_hTargetEntity->GetAbsOrigin() - GetAbsOrigin();
				VectorAngles( vecToTarget, vecAngles );
			}
			AngleVectors( vecAngles, &vForward, &vRight, &vUp );
		}
	}
	else
	{
		AngleVectors( GetAbsAngles(), &vForward, &vRight, &vUp );
	}


	m_FlashlightState.m_fHorizontalFOVDegrees = m_flLightFOV;
	m_FlashlightState.m_fVerticalFOVDegrees = m_flLightFOV;

	m_FlashlightState.m_vecLightOrigin = vPos;
	BasisToQuaternion( vForward, vRight, vUp, m_FlashlightState.m_quatOrientation );

	m_FlashlightState.m_fQuadraticAtten = 0.0;
	m_FlashlightState.m_fLinearAtten = 100;
	m_FlashlightState.m_fConstantAtten = 0.0f;
	m_FlashlightState.m_Color[0] = m_LinearFloatLightColor.x;
	m_FlashlightState.m_Color[1] = m_LinearFloatLightColor.y;
	m_FlashlightState.m_Color[2] = m_LinearFloatLightColor.z;
	m_FlashlightState.m_Color[3] = 0.0f; // fixme: need to make ambient work m_flAmbient;
	m_FlashlightState.m_NearZ = m_flNearZ;
	m_FlashlightState.m_FarZ = m_flFarZ;

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
	Vector mins = static_cast<Vector>(**vOutRects);
	Vector maxs = static_cast<Vector>(**vOutRects);
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

	m_FlashlightState.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap.GetFloat();
	m_FlashlightState.m_flShadowDepthBias = mat_depthbias_shadowmap.GetFloat();
	m_FlashlightState.m_bEnableShadows = m_bEnableShadows;
	m_FlashlightState.m_pSpotlightTexture = materials->FindTexture( m_SpotlightTextureName, TEXTURE_GROUP_OTHER, false );
	m_FlashlightState.m_nSpotlightTextureFrame = m_nSpotlightTextureFrame;

	m_FlashlightState.m_nShadowQuality = m_nShadowQuality; // Allow entity to affect shadow quality

	if ( m_LightHandle == CLIENTSHADOW_INVALID_HANDLE )
	{
		m_LightHandle = g_pClientShadowMgr->CreateFlashlight( m_FlashlightState );
	}
	else
	{
		if ( m_hTargetEntity != NULL || bForceUpdate )
		{
			g_pClientShadowMgr->UpdateFlashlightState( m_LightHandle, m_FlashlightState );
		}
	}

	if ( m_bLightOnlyTarget )
	{
		g_pClientShadowMgr->SetFlashlightTarget( m_LightHandle, m_hTargetEntity );
	}
	else
	{
		g_pClientShadowMgr->SetFlashlightTarget( m_LightHandle, NULL );
	}

	g_pClientShadowMgr->SetFlashlightLightWorld( m_LightHandle, m_bLightWorld );

	g_pClientShadowMgr->UpdateProjectedTexture( m_LightHandle, true );

	if ( m_bEnableVolumetrics && m_bEnableShadows )
	{
		CViewSetup setup;
		GetShadowViewSetup( setup );

		VMatrix world2View, view2Proj, world2Proj, world2Pixels;
		render->GetMatricesForView( setup, &world2View, &view2Proj, &world2Proj, &world2Pixels );
		VMatrix proj2world;
		MatrixInverseGeneral( world2Proj, proj2world );

		Vector fwd, right, up;
		AngleVectors( setup.angles, &fwd, &right, &up );

		Vector nearFarPlane[8];
		Vector3DMultiplyPositionProjective( proj2world, Vector( -1, -1, 1 ), nearFarPlane[ 0 ] );
		Vector3DMultiplyPositionProjective( proj2world, Vector( 1, -1, 1 ), nearFarPlane[ 1 ] );
		Vector3DMultiplyPositionProjective( proj2world, Vector( 1, 1, 1 ), nearFarPlane[ 2 ] );
		Vector3DMultiplyPositionProjective( proj2world, Vector( -1, 1, 1 ), nearFarPlane[ 3 ] );

		Vector3DMultiplyPositionProjective( proj2world, Vector( -1, -1, 0 ), nearFarPlane[ 4 ] );
		Vector3DMultiplyPositionProjective( proj2world, Vector( 1, -1, 0 ), nearFarPlane[ 5 ] );
		Vector3DMultiplyPositionProjective( proj2world, Vector( 1, 1, 0 ), nearFarPlane[ 6 ] );
		Vector3DMultiplyPositionProjective( proj2world, Vector( -1, 1, 0 ), nearFarPlane[ 7 ] );

		m_vecRenderBoundsMin.Init( MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT );
		m_vecRenderBoundsMax.Init( MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT );

		for ( int i = 0; i < 8; i++ )
		{
			m_vecRenderBoundsMin.x = Min( m_vecRenderBoundsMin.x, nearFarPlane[ i ].x );
			m_vecRenderBoundsMin.y = Min( m_vecRenderBoundsMin.y, nearFarPlane[ i ].y );
			m_vecRenderBoundsMin.z = Min( m_vecRenderBoundsMin.y, nearFarPlane[ i ].z );
			m_vecRenderBoundsMax.x = Max( m_vecRenderBoundsMax.x, nearFarPlane[ i ].x );
			m_vecRenderBoundsMax.y = Max( m_vecRenderBoundsMax.y, nearFarPlane[ i ].y );
			m_vecRenderBoundsMax.z = Max( m_vecRenderBoundsMax.y, nearFarPlane[ i ].z );
		}
	}
}

void C_EnvProjectedTexture::Simulate( void )
{
	UpdateLight( GetMoveParent() != NULL );

	BaseClass::Simulate();
}

int C_EnvProjectedTexture::DrawModel( int flags )
{
	if ( !m_bState ||
		m_LightHandle == CLIENTSHADOW_INVALID_HANDLE ||
		!m_bEnableVolumetrics ||
		!volumetrics_enabled.GetBool() ||
		CurrentViewID() != VIEW_MAIN )
	{
		return 0;
	}

	float flDistanceFade = 1.0f;
	if ( m_flVolumetricsFadeDistance > 0.0f )
	{
		Vector delta = CurrentViewOrigin() - GetLocalOrigin();
		flDistanceFade = RemapValClamped( delta.Length(),
			m_flVolumetricsFadeDistance + volumetrics_fade_range.GetFloat(),
			m_flVolumetricsFadeDistance, 0.0f, 1.0f );
	}

	if ( flDistanceFade <= 0.0f )
	{
		return 0;
	}

	ITexture *pDepthTexture = NULL;
	const ShadowHandle_t shadowHandle = g_pClientShadowMgr->GetShadowHandle( m_LightHandle );
	const int iNumActiveDepthTextures = g_pClientShadowMgr->GetNumShadowDepthtextures();
	for ( int i = 0; i < iNumActiveDepthTextures; i++)
	{
		if ( g_pClientShadowMgr->GetShadowDepthHandle( i ) == shadowHandle )
		{
			pDepthTexture = g_pClientShadowMgr->GetShadowDepthTex( i );
			break;
		}
	}

	if ( pDepthTexture == NULL )
	{
		return 0;
	}

	if ( m_pVolmetricMesh == NULL )
	{
		RebuildVolumetricMesh();
	}

	UpdateScreenEffectTexture();

	CViewSetup setup;
	GetShadowViewSetup( setup );

	VMatrix world2View, view2Proj, world2Proj, world2Pixels;
	render->GetMatricesForView( setup, &world2View, &view2Proj, &world2Proj, &world2Pixels );

	VMatrix tmp, shadowToUnit;
	MatrixBuildScale( tmp, 1.0f / 2, 1.0f / -2, 1.0f );
	tmp[0][3] = tmp[1][3] = 0.5f;
	MatrixMultiply( tmp, world2Proj, shadowToUnit );

	m_FlashlightState.m_Color[ 3 ] = m_flVolumetricsMultiplier / m_iCurrentVolumetricsSubDiv * flDistanceFade;

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->SetFlashlightMode( true );
	pRenderContext->SetFlashlightStateEx( m_FlashlightState, shadowToUnit, pDepthTexture );

	matrix3x4_t flashlightTransform;
	AngleMatrix( setup.angles, setup.origin, flashlightTransform );

	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->PushMatrix();
	pRenderContext->LoadMatrix( flashlightTransform );

	pRenderContext->Bind( m_matVolumetricsMaterial );
	m_pVolmetricMesh->Draw();

	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->PopMatrix();
	pRenderContext->SetFlashlightMode( false );
	return 1;
}

void C_EnvProjectedTexture::ClearVolumetricsMesh()
{
	if ( m_pVolmetricMesh != NULL )
	{
		CMatRenderContextPtr pRenderContext( materials );
		pRenderContext->DestroyStaticMesh( m_pVolmetricMesh );
		m_pVolmetricMesh = NULL;
	}
}

void C_EnvProjectedTexture::RebuildVolumetricMesh()
{
	if ( !m_matVolumetricsMaterial.IsValid() )
		m_matVolumetricsMaterial.Init( "engine/light_volumetrics", TEXTURE_GROUP_OTHER );

	ClearVolumetricsMesh();

	CViewSetup setup;
	GetShadowViewSetup( setup );
	setup.origin = vec3_origin;
	setup.angles = vec3_angle;

	VMatrix world2View, view2Proj, world2Proj, world2Pixels;
	render->GetMatricesForView( setup, &world2View, &view2Proj, &world2Proj, &world2Pixels );
	VMatrix proj2world;
	MatrixInverseGeneral( world2Proj, proj2world );

	const Vector origin( vec3_origin );
	QAngle angles( vec3_angle );
	Vector fwd, right, up;
	AngleVectors( angles, &fwd, &right, &up );

	Vector nearPlane[4];
	Vector farPlane[4];
	Vector3DMultiplyPositionProjective( proj2world, Vector( -1, -1, 1 ), farPlane[ 0 ] );
	Vector3DMultiplyPositionProjective( proj2world, Vector( 1, -1, 1 ), farPlane[ 1 ] );
	Vector3DMultiplyPositionProjective( proj2world, Vector( 1, 1, 1 ), farPlane[ 2 ] );
	Vector3DMultiplyPositionProjective( proj2world, Vector( -1, 1, 1 ), farPlane[ 3 ] );

	Vector3DMultiplyPositionProjective( proj2world, Vector( -1, -1, 0 ), nearPlane[ 0 ] );
	Vector3DMultiplyPositionProjective( proj2world, Vector( 1, -1, 0 ), nearPlane[ 1 ] );
	Vector3DMultiplyPositionProjective( proj2world, Vector( 1, 1, 0 ), nearPlane[ 2 ] );
	Vector3DMultiplyPositionProjective( proj2world, Vector( -1, 1, 0 ), nearPlane[ 3 ] );

	DebugDrawLine( farPlane[ 0 ], farPlane[ 1 ], 0, 0, 255, true, -1 );
	DebugDrawLine( farPlane[ 1 ], farPlane[ 2 ], 0, 0, 255, true, -1 );
	DebugDrawLine( farPlane[ 2 ], farPlane[ 3 ], 0, 0, 255, true, -1 );
	DebugDrawLine( farPlane[ 3 ], farPlane[ 0 ], 0, 0, 255, true, -1 );
	DebugDrawLine( nearPlane[ 0 ], nearPlane[ 1 ], 0, 0, 255, true, -1 );
	DebugDrawLine( nearPlane[ 1 ], nearPlane[ 2 ], 0, 0, 255, true, -1 );
	DebugDrawLine( nearPlane[ 2 ], nearPlane[ 3 ], 0, 0, 255, true, -1 );
	DebugDrawLine( nearPlane[ 3 ], nearPlane[ 0 ], 0, 0, 255, true, -1 );
	DebugDrawLine( farPlane[ 0 ], nearPlane[ 0 ], 0, 0, 255, true, -1 );
	DebugDrawLine( farPlane[ 1 ], nearPlane[ 1 ], 0, 0, 255, true, -1 );
	DebugDrawLine( farPlane[ 2 ], nearPlane[ 2 ], 0, 0, 255, true, -1 );
	DebugDrawLine( farPlane[ 3 ], nearPlane[ 3 ], 0, 0, 255, true, -1 );

	const Vector vecDirections[3] = {
		( farPlane[ 0 ] - vec3_origin ),
		( farPlane[ 1 ] - farPlane[ 0 ] ),
		( farPlane[ 3 ] - farPlane[ 0 ] ),
	};

	const VertexFormat_t vertexFormat =  VERTEX_POSITION | VERTEX_TEXCOORD_SIZE( 0, 2 );
	CMatRenderContextPtr pRenderContext( materials );
	m_pVolmetricMesh = pRenderContext->CreateStaticMesh( vertexFormat, TEXTURE_GROUP_OTHER, m_matVolumetricsMaterial );

	const int iCvarSubDiv = volumetrics_subdiv.GetInt();
	m_iCurrentVolumetricsSubDiv = ( iCvarSubDiv > 2 ) ? iCvarSubDiv : Max( m_iVolumetricsQuality, 3 );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( m_pVolmetricMesh, MATERIAL_TRIANGLES, m_iCurrentVolumetricsSubDiv * 6 - 4 );

	for ( int x = 1; x < m_iCurrentVolumetricsSubDiv * 2; x++ )
	{
		// never 0.0 or 1.0
		float flFracX = x / float( m_iCurrentVolumetricsSubDiv * 2 );
		//flFracX = powf( flFracX, 3.0f );
		flFracX = powf( flFracX, m_flVolumetricsQualityBias );

		Vector v00 = origin + vecDirections[ 0 ] * flFracX;
		Vector v10 = v00 + vecDirections[ 1 ] * flFracX;
		Vector v11 = v10 + vecDirections[ 2 ] * flFracX;
		Vector v01 = v00 + vecDirections[ 2 ] * flFracX;

		meshBuilder.Position3f( XYZ( v00 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v10 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v11 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v00 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v11 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v01 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();
	}

	for ( int x = 1; x < m_iCurrentVolumetricsSubDiv; x++ )
	{
		// never 0.0 or 1.0
		const float flFracX = x / float( m_iCurrentVolumetricsSubDiv );
		Vector v0 = origin + vecDirections[ 0 ] + vecDirections[ 1 ] * flFracX;
		Vector v1 = v0 + vecDirections[ 2 ];

		meshBuilder.Position3f( XYZ( origin ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v0 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v1 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();
	}

	for ( int x = 1; x < m_iCurrentVolumetricsSubDiv; x++ )
	{
		// never 0.0 or 1.0
		const float flFracX = x / float( m_iCurrentVolumetricsSubDiv );
		Vector v0 = origin + vecDirections[ 0 ] + vecDirections[ 2 ] * flFracX;
		Vector v1 = v0 + vecDirections[ 1 ];

		meshBuilder.Position3f( XYZ( origin ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v0 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( XYZ( v1 ) );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.AdvanceVertex();
	}

	meshBuilder.End();
}

void C_EnvProjectedTexture::GetShadowViewSetup( CViewSetup &setup )
{
	setup.origin = m_FlashlightState.m_vecLightOrigin;
	QuaternionAngles( m_FlashlightState.m_quatOrientation, setup.angles );
	setup.fov = m_flLightFOV;
	setup.zFar = m_flFarZ;
	setup.zNear = m_flNearZ;
	setup.m_bOrtho = false;
	setup.m_flAspectRatio = 1.0f;
	setup.x = setup.y = 0;
	setup.width = m_FlashlightState.m_pSpotlightTexture ? m_FlashlightState.m_pSpotlightTexture->GetActualWidth() : 512;
	setup.height = m_FlashlightState.m_pSpotlightTexture ? m_FlashlightState.m_pSpotlightTexture->GetActualHeight() : 512;
}
