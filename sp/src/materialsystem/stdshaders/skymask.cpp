#include "basevsshader.h"

#include "skymask_vs20.inc"
#include "skymask_ps20b.inc"
#include "fbquad_vs20.inc"
#include "fbquad_ps20b.inc"


BEGIN_VS_SHADER( fill_viewmask, "" )
	BEGIN_SHADER_PARAMS
	SHADER_PARAM( BASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "" )
	SHADER_PARAM( COMBINEMODE, SHADER_PARAM_TYPE_INTEGER, "", "" )
	SHADER_PARAM( VIEWMODEL, SHADER_PARAM_TYPE_BOOL, "", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		if ( params[COMBINEMODE]->GetIntValue() )
			LoadTexture( BASETEXTURE );
		if ( params[COMBINEMODE]->GetIntValue() > 1 )
			LoadTexture( BASETEXTURE2 );
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_DRAW
	{
		const int iCombineMode = params[COMBINEMODE]->GetIntValue();
		const bool bViewModel = !!params[VIEWMODEL]->GetIntValue();
	
		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();
	
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( false );
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableCulling( false );
	
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );
	
			if ( iCombineMode )
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			if ( iCombineMode > 1 )
				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			DECLARE_STATIC_VERTEX_SHADER( skymask_vs20 );
			SET_STATIC_VERTEX_SHADER_COMBO( DRAWWHITE, false );
			SET_STATIC_VERTEX_SHADER_COMBO( COMBINE, iCombineMode );
			SET_STATIC_VERTEX_SHADER_COMBO( VIEWMODEL, bViewModel );
			SET_STATIC_VERTEX_SHADER( skymask_vs20 );
				

			DECLARE_STATIC_PIXEL_SHADER( skymask_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO( DRAWWHITE, false );
			SET_STATIC_PIXEL_SHADER_COMBO( COMBINE, iCombineMode );
			SET_STATIC_PIXEL_SHADER_COMBO( VIEWMODEL, bViewModel );
			SET_STATIC_PIXEL_SHADER( skymask_ps20b );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( 0 );

			if ( iCombineMode )
				BindTexture( SHADER_SAMPLER0,  BASETEXTURE, -1 );
			if ( iCombineMode > 1 )
				BindTexture( SHADER_SAMPLER1,  BASETEXTURE2, -1 );
		}
		Draw( true );
	
		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();
	
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( true );
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableCulling( false );
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );
	
			if ( iCombineMode )
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			if ( iCombineMode > 1 )
				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			DECLARE_STATIC_VERTEX_SHADER( skymask_vs20 );
			SET_STATIC_VERTEX_SHADER_COMBO( DRAWWHITE, true );
			SET_STATIC_VERTEX_SHADER_COMBO( COMBINE, iCombineMode );
			SET_STATIC_VERTEX_SHADER_COMBO( VIEWMODEL, bViewModel );
			SET_STATIC_VERTEX_SHADER( skymask_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( skymask_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO( DRAWWHITE, true );
			SET_STATIC_PIXEL_SHADER_COMBO( COMBINE, iCombineMode );
			SET_STATIC_PIXEL_SHADER_COMBO( VIEWMODEL, bViewModel );
			SET_STATIC_PIXEL_SHADER( skymask_ps20b );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();
			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( 0 );

			if ( iCombineMode )
				BindTexture( SHADER_SAMPLER0,  BASETEXTURE, -1 );
			if ( iCombineMode > 1 )
				BindTexture( SHADER_SAMPLER1,  BASETEXTURE2, -1 );
		}
		Draw( true );
	}
END_SHADER

BEGIN_VS_SHADER( FULLSCREENQUAD_WRITEA, "" )
	BEGIN_SHADER_PARAMS
	SHADER_PARAM( COMBINEMODE, SHADER_PARAM_TYPE_BOOL, "", "" )
	SHADER_PARAM( BASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
		if ( params[COMBINEMODE]->GetIntValue() )
			LoadTexture( BASETEXTURE2 );
	}
		
	SHADER_FALLBACK
	{
		return 0;
	}
		
	SHADER_DRAW
	{
		const bool bCombineMode = !!params[COMBINEMODE]->GetIntValue();

		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();

			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( true );
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( true );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			if ( bCombineMode )
				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0 );

			DECLARE_STATIC_VERTEX_SHADER( fbquad_vs20 );
			SET_STATIC_VERTEX_SHADER( fbquad_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( fbquad_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO( DRAWCOLOR, 0 );
			SET_STATIC_PIXEL_SHADER_COMBO( COMBINEMODE, bCombineMode );
			SET_STATIC_PIXEL_SHADER( fbquad_ps20b );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( 0 );

			BindTexture( SHADER_SAMPLER0,  BASETEXTURE );
			if ( bCombineMode )
				BindTexture( SHADER_SAMPLER1,  BASETEXTURE2 );
		}
		Draw();
	}
END_SHADER