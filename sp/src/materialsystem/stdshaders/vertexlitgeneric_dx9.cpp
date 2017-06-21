//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=====================================================================================//

#include "BaseVSShader.h"
#include "vertexlitgeneric_dx9_helper.h"


BEGIN_VS_SHADER( VertexLitTreeSway, "Help for VertexLitTreeSway" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)" )
		SHADER_PARAM( COMPRESS, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "compression wrinklemap" )
		SHADER_PARAM( STRETCH, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "expansion wrinklemap" )
		SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture" )
		SHADER_PARAM( DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail" )
		SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number" )
		SHADER_PARAM( ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask" )
		SHADER_PARAM( ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map" )
		SHADER_PARAM( BUMPCOMPRESS, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader3_normal", "compression bump map" )
		SHADER_PARAM( BUMPSTRETCH, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "expansion bump map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
 	    SHADER_PARAM( SELFILLUM_ENVMAPMASK_ALPHA, SHADER_PARAM_TYPE_FLOAT,"0.0","defines that self illum value comes from env map mask alpha" )
		SHADER_PARAM( SELFILLUMFRESNEL, SHADER_PARAM_TYPE_BOOL, "0", "Self illum fresnel" )
		SHADER_PARAM( SELFILLUMFRESNELMINMAXEXP, SHADER_PARAM_TYPE_VEC4, "0", "Self illum fresnel min, max, exp" )
		SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )	
		SHADER_PARAM( FLASHLIGHTNOLAMBERT, SHADER_PARAM_TYPE_BOOL, "0", "Flashlight pass sets N.L=1.0" )

		// Debugging term for visualizing ambient data on its own
		SHADER_PARAM( AMBIENTONLY, SHADER_PARAM_TYPE_INTEGER, "0", "Control drawing of non-ambient light ()" )

		SHADER_PARAM( PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "5.0", "Phong exponent for local specular lights" )
		SHADER_PARAM( PHONGTINT, SHADER_PARAM_TYPE_VEC3, "5.0", "Phong tint for local specular lights" )
		SHADER_PARAM( PHONGALBEDOTINT, SHADER_PARAM_TYPE_BOOL, "1.0", "Apply tint by albedo (controlled by spec exponent texture" )
		SHADER_PARAM( LIGHTWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for tinting scalar diffuse term" )
		SHADER_PARAM( PHONGWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "warp the specular term" )
		SHADER_PARAM( PHONGFRESNELRANGES, SHADER_PARAM_TYPE_VEC3, "[0  0.5  1]", "Parameters for remapping fresnel output" )
		SHADER_PARAM( PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Phong overbrightening factor (specular mask channel should be authored to account for this)" )
		SHADER_PARAM( PHONGEXPONENTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Phong Exponent map" )
		SHADER_PARAM( PHONG, SHADER_PARAM_TYPE_BOOL, "0", "enables phong lighting" )
		SHADER_PARAM( BASEMAPALPHAPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "indicates that there is no normal map and that the phong mask is in base alpha" )
		SHADER_PARAM( INVERTPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "invert the phong mask (0=full phong, 1=no phong)" )
		SHADER_PARAM( ENVMAPFRESNEL, SHADER_PARAM_TYPE_FLOAT, "0", "Degree to which Fresnel should be applied to env map" )
		SHADER_PARAM( SELFILLUMMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "If we bind a texture here, it overrides base alpha (if any) for self illum" )

	    // detail (multi-) texturing
	    SHADER_PARAM( DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "mode for combining detail texture with base. 0=normal, 1= additive, 2=alpha blend detail over base, 3=crossfade" )
		SHADER_PARAM( DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture." )
		SHADER_PARAM( DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint" )
		SHADER_PARAM( DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail texcoord transform" )

		// Rim lighting terms
		SHADER_PARAM( RIMLIGHT, SHADER_PARAM_TYPE_BOOL, "0", "enables rim lighting" )
		SHADER_PARAM( RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "4.0", "Exponent for rim lights" )
		SHADER_PARAM( RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Boost for rim lights" )
		SHADER_PARAM( RIMMASK, SHADER_PARAM_TYPE_BOOL, "0", "Indicates whether or not to use alpha channel of exponent texture to mask the rim term" )

	    // Seamless mapping scale
		SHADER_PARAM( SEAMLESS_BASE, SHADER_PARAM_TYPE_BOOL, "0", "whether to apply seamless mapping to the base texture. requires a smooth model." )
		SHADER_PARAM( SEAMLESS_DETAIL, SHADER_PARAM_TYPE_BOOL, "0", "where to apply seamless mapping to the detail texture." )
	    SHADER_PARAM( SEAMLESS_SCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "the scale for the seamless mapping. # of repetions of texture per inch." )

		SHADER_PARAM( SEPARATEDETAILUVS, SHADER_PARAM_TYPE_BOOL, "0", "Use texcoord1 for detail texture" )
		SHADER_PARAM( LINEARWRITE, SHADER_PARAM_TYPE_INTEGER, "0", "Disables SRGB conversion of shader results." )
		SHADER_PARAM( DEPTHBLEND, SHADER_PARAM_TYPE_INTEGER, "0", "fade at intersection boundaries. Only supported without bumpmaps" )
		SHADER_PARAM( DEPTHBLENDSCALE, SHADER_PARAM_TYPE_FLOAT, "50.0", "Amplify or reduce DEPTHBLEND fading. Lower values make harder edges." )

		SHADER_PARAM( BLENDTINTBYBASEALPHA, SHADER_PARAM_TYPE_BOOL, "0", "Use the base alpha to blend in the $color modulation")
		SHADER_PARAM( BLENDTINTCOLOROVERBASE, SHADER_PARAM_TYPE_FLOAT, "0", "blend between tint acting as a multiplication versus a replace" )

		// vertexlitgeneric tree sway animation control
		SHADER_PARAM( TREESWAY, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( TREESWAYHEIGHT, SHADER_PARAM_TYPE_FLOAT, "1000", "" )
		SHADER_PARAM( TREESWAYSTARTHEIGHT, SHADER_PARAM_TYPE_FLOAT, "0.2", "" )
		SHADER_PARAM( TREESWAYRADIUS, SHADER_PARAM_TYPE_FLOAT, "300", "" )
		SHADER_PARAM( TREESWAYSTARTRADIUS, SHADER_PARAM_TYPE_FLOAT, "0.1", "" )
		SHADER_PARAM( TREESWAYSPEED, SHADER_PARAM_TYPE_FLOAT, "1", "" )
		SHADER_PARAM( TREESWAYSPEEDHIGHWINDMULTIPLIER, SHADER_PARAM_TYPE_FLOAT, "2", "" )
		SHADER_PARAM( TREESWAYSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "10", "" )
		SHADER_PARAM( TREESWAYSCRUMBLESPEED, SHADER_PARAM_TYPE_FLOAT, "0.1", "" )
		SHADER_PARAM( TREESWAYSCRUMBLESTRENGTH, SHADER_PARAM_TYPE_FLOAT, "0.1", "" )
		SHADER_PARAM( TREESWAYSCRUMBLEFREQUENCY, SHADER_PARAM_TYPE_FLOAT, "0.1", "" )
		SHADER_PARAM( TREESWAYFALLOFFEXP, SHADER_PARAM_TYPE_FLOAT, "1.5", "" )
		SHADER_PARAM( TREESWAYSCRUMBLEFALLOFFEXP, SHADER_PARAM_TYPE_FLOAT, "1.0", "" )
		SHADER_PARAM( TREESWAYSPEEDLERPSTART, SHADER_PARAM_TYPE_FLOAT, "3", "" )
		SHADER_PARAM( TREESWAYSPEEDLERPEND, SHADER_PARAM_TYPE_FLOAT, "6", "" )

	END_SHADER_PARAMS

	void SetupVars( VertexLitGeneric_DX9_Vars_t& info )
	{
		info.m_nBaseTexture = BASETEXTURE;
		info.m_nWrinkle = COMPRESS;
		info.m_nStretch = STRETCH;
		info.m_nBaseTextureFrame = FRAME;
		info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
		info.m_nAlbedo = ALBEDO;
		info.m_nSelfIllumTint = SELFILLUMTINT;
		info.m_nDetail = DETAIL;
		info.m_nDetailFrame = DETAILFRAME;
		info.m_nDetailScale = DETAILSCALE;
		info.m_nEnvmap = ENVMAP;
		info.m_nEnvmapFrame = ENVMAPFRAME;
		info.m_nEnvmapMask = ENVMAPMASK;
		info.m_nEnvmapMaskFrame = ENVMAPMASKFRAME;
		info.m_nEnvmapMaskTransform = ENVMAPMASKTRANSFORM;
		info.m_nEnvmapTint = ENVMAPTINT;
		info.m_nBumpmap = BUMPMAP;
		info.m_nNormalWrinkle = BUMPCOMPRESS;
		info.m_nNormalStretch = BUMPSTRETCH;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpTransform = BUMPTRANSFORM;
		info.m_nEnvmapContrast = ENVMAPCONTRAST;
		info.m_nEnvmapSaturation = ENVMAPSATURATION;
		info.m_nAlphaTestReference = ALPHATESTREFERENCE;
		info.m_nFlashlightNoLambert = FLASHLIGHTNOLAMBERT;

		info.m_nFlashlightTexture = FLASHLIGHTTEXTURE;
		info.m_nFlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
		info.m_nSelfIllumEnvMapMask_Alpha = SELFILLUM_ENVMAPMASK_ALPHA;
		info.m_nSelfIllumFresnel = SELFILLUMFRESNEL;
		info.m_nSelfIllumFresnelMinMaxExp = SELFILLUMFRESNELMINMAXEXP;

		info.m_nAmbientOnly = AMBIENTONLY;
		info.m_nPhongExponent = PHONGEXPONENT;
		info.m_nPhongExponentTexture = PHONGEXPONENTTEXTURE;
		info.m_nPhongTint = PHONGTINT;
		info.m_nPhongAlbedoTint = PHONGALBEDOTINT;
		info.m_nDiffuseWarpTexture = LIGHTWARPTEXTURE;
		info.m_nPhongWarpTexture = PHONGWARPTEXTURE;
		info.m_nPhongBoost = PHONGBOOST;
		info.m_nPhongFresnelRanges = PHONGFRESNELRANGES;
		info.m_nPhong = PHONG;
		info.m_nBaseMapAlphaPhongMask = BASEMAPALPHAPHONGMASK;
		info.m_nEnvmapFresnel = ENVMAPFRESNEL;
		info.m_nDetailTextureCombineMode = DETAILBLENDMODE;
		info.m_nDetailTextureBlendFactor = DETAILBLENDFACTOR;
		info.m_nDetailTextureTransform = DETAILTEXTURETRANSFORM;

		// Rim lighting parameters
		info.m_nRimLight = RIMLIGHT;
		info.m_nRimLightPower = RIMLIGHTEXPONENT;
		info.m_nRimLightBoost = RIMLIGHTBOOST;
		info.m_nRimMask = RIMMASK;

		// seamless
		info.m_nSeamlessScale = SEAMLESS_SCALE;
		info.m_nSeamlessDetail = SEAMLESS_DETAIL;
		info.m_nSeamlessBase = SEAMLESS_BASE;

		info.m_nSeparateDetailUVs = SEPARATEDETAILUVS;

		info.m_nLinearWrite = LINEARWRITE;
		info.m_nDetailTint = DETAILTINT;
		info.m_nInvertPhongMask = INVERTPHONGMASK;

		info.m_nDepthBlend = DEPTHBLEND;
		info.m_nDepthBlendScale = DEPTHBLENDSCALE;

		info.m_nSelfIllumMask = SELFILLUMMASK;
		info.m_nBlendTintByBaseAlpha = BLENDTINTBYBASEALPHA;
		info.m_nTintReplacesBaseColor = BLENDTINTCOLOROVERBASE;

		info.m_nTreeSway = TREESWAY;
		info.m_nTreeSwayHeight = TREESWAYHEIGHT;
		info.m_nTreeSwayStartHeight = TREESWAYSTARTHEIGHT;
		info.m_nTreeSwayRadius = TREESWAYRADIUS;
		info.m_nTreeSwayStartRadius = TREESWAYSTARTRADIUS;
		info.m_nTreeSwaySpeed = TREESWAYSPEED;
		info.m_nTreeSwaySpeedHighWindMultiplier = TREESWAYSPEEDHIGHWINDMULTIPLIER;
		info.m_nTreeSwayStrength = TREESWAYSTRENGTH;
		info.m_nTreeSwayScrumbleSpeed = TREESWAYSCRUMBLESPEED;
		info.m_nTreeSwayScrumbleStrength = TREESWAYSCRUMBLESTRENGTH;
		info.m_nTreeSwayScrumbleFrequency = TREESWAYSCRUMBLEFREQUENCY;
		info.m_nTreeSwayFalloffExp = TREESWAYFALLOFFEXP;
		info.m_nTreeSwayScrumbleFalloffExp = TREESWAYSCRUMBLEFALLOFFEXP;
		info.m_nTreeSwaySpeedLerpStart = TREESWAYSPEEDLERPSTART;
		info.m_nTreeSwaySpeedLerpEnd = TREESWAYSPEEDLERPEND;
	}

	bool IsTranslucent( IMaterialVar **params ) const
	{
		// Check flag if not drawing cloak pass
		return IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ); 
	}

	SHADER_INIT_PARAMS()
	{
		VertexLitGeneric_DX9_Vars_t vars;
		SetupVars( vars );
		InitParamsVertexLitGeneric_DX9( this, params, pMaterialName, true, vars );
	}

	SHADER_FALLBACK
	{
		if ( CShader_IsFlag2Set( params, MATERIAL_VAR2_USE_EDITOR ) )
			return "VertexLitGeneric";
		return 0;
	}

	SHADER_INIT
	{
		VertexLitGeneric_DX9_Vars_t vars;
		SetupVars( vars );
		InitVertexLitGeneric_DX9( this, params, true, vars );
	}

	SHADER_DRAW
	{
		VertexLitGeneric_DX9_Vars_t vars;
		SetupVars( vars );
		DrawVertexLitGeneric_DX9( this, params, pShaderAPI, pShaderShadow, true, vars, vertexCompression, pContextDataPtr );
	}
END_SHADER
