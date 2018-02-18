#include "basevsshader.h"

#include "sunrays_calc_vs20.inc"
#include "sunrays_calc_ps20b.inc"

BEGIN_VS_SHADER( SS_Sunrays, "Help for SS_Sunrays" ) //Begin the shader
	
	BEGIN_SHADER_PARAMS
	SHADER_PARAM(SUNX, SHADER_PARAM_TYPE_FLOAT, "0", "")
	SHADER_PARAM(SUNY, SHADER_PARAM_TYPE_FLOAT, "0", "")
	SHADER_PARAM(SUNZ, SHADER_PARAM_TYPE_FLOAT, "0", "")
	SHADER_PARAM(SUNCOLOR, SHADER_PARAM_TYPE_VEC3, "0", "")
	SHADER_PARAM(SUNSCALE, SHADER_PARAM_TYPE_INTEGER, "2", "" )	//default is 2.
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS() //called after parameters have been initalised
	{
	}

	SHADER_FALLBACK //dosnt fallback to anything (I know this works on dx9, hasnt been tested on others)
	{
		return 0;
	}

	//bool NeedsFullFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame ) const
	//{
	//	return true;
	//}

	SHADER_INIT //initializes the shader
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

				pShaderShadow->EnableDepthWrites( false );
				pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0);

				DECLARE_STATIC_VERTEX_SHADER( sunrays_calc_vs20 );
				SET_STATIC_VERTEX_SHADER( sunrays_calc_vs20 );

				DECLARE_STATIC_PIXEL_SHADER( sunrays_calc_ps20b );
				SET_STATIC_PIXEL_SHADER( sunrays_calc_ps20b );
			}
		DYNAMIC_STATE
			{
				BindTexture( SHADER_SAMPLER0,  BASETEXTURE );

				float vConst[3] = { 0.0f, 0.0f, 0.0f };

				vConst[0] = params[SUNX]->GetFloatValue();
				vConst[1] = params[SUNY]->GetFloatValue();
				vConst[2] = params[SUNZ]->GetFloatValue();

				pShaderAPI->SetPixelShaderConstant(0, vConst);
				SetPixelShaderConstant( 1, SUNCOLOR );

				DECLARE_DYNAMIC_PIXEL_SHADER( sunrays_calc_ps20b );
				SET_DYNAMIC_PIXEL_SHADER_COMBO( INTENSITY, params[SUNSCALE]->GetIntValue() );
				SET_DYNAMIC_PIXEL_SHADER( sunrays_calc_ps20b );
			}
		Draw(); //draw the shader
	}

END_SHADER
