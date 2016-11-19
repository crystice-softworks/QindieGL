/***************************************************************************
* Copyright (C) 2011-2016, Crystice Softworks.
* 
* This file is part of QindieGL source code.
* Please note that QindieGL is not driver, it's emulator.
* 
* QindieGL source code is free software; you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as 
* published by the Free Software Foundation; either version 2 of 
* the License, or (at your option) any later version.
* 
* QindieGL source code is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied 
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
***************************************************************************/
#include "d3d_wrapper.hpp"
#include "d3d_global.hpp"
#include "d3d_state.hpp"

//==================================================================================
// TNT Combiners
//----------------------------------------------------------------------------------
// TODO: additional combiner modes
//==================================================================================
typedef struct stTNTCombinerReplacement
{
	int numTNTCombiners;
	int numD3DCombiners;
	struct {
		GLenum		envMode;
		DWORD		colorScale;
		DWORD		alphaScale;
		GLenum		colorOp;
		GLenum		colorArg1;
		GLenum		colorArg2;
		GLenum		colorArg3;
		GLenum		colorOperand1;
		GLenum		colorOperand2;
		GLenum		colorOperand3;
		GLenum		alphaOp;
		GLenum		alphaArg1;
		GLenum		alphaArg2;
		GLenum		alphaArg3;
		GLenum		alphaOperand1;
		GLenum		alphaOperand2;
		GLenum		alphaOperand3;
	} TNTCombiner[MAX_D3D_TMU];
	struct {
		DWORD		colorOp;
		DWORD		colorArg1;
		DWORD		colorArg2;
		DWORD		colorArg3;
		DWORD		alphaOp;
		DWORD		alphaArg1;
		DWORD		alphaArg2;
		DWORD		alphaArg3;
		DWORD		resultArg;
	} D3DCombiner[MAX_D3D_TMU];
} TNTCombinerReplacement;

static TNTCombinerReplacement s_TNTCombinerReplacements[] = {
// Doomsday interpolation combiner shader fix
{ 2, 3,
// GL Combiner desc
{{ GL_COMBINE_ARB, 1, 1, 
  GL_INTERPOLATE_ARB, GL_TEXTURE1_ARB, GL_TEXTURE0_ARB, GL_CONSTANT_ARB, GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA,
  GL_REPLACE, GL_PREVIOUS_ARB, GL_PREVIOUS_ARB, GL_CONSTANT_ARB, GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA },
 { GL_COMBINE_ARB, 1, 1, 
  GL_MODULATE, GL_PRIMARY_COLOR_ARB, GL_PREVIOUS_ARB, GL_CONSTANT_ARB, GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA,
  GL_REPLACE, GL_PREVIOUS_ARB, GL_PREVIOUS_ARB, GL_CONSTANT_ARB, GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA }},
// D3D Combiner desc
{{ D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE, D3DTA_CURRENT,
   D3DTOP_SELECTARG1, D3DTA_CURRENT, D3DTA_CURRENT, D3DTA_CURRENT, 
   D3DTA_CURRENT },
 { D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE, D3DTA_CURRENT,
   D3DTOP_SELECTARG1, D3DTA_CURRENT, D3DTA_CURRENT, D3DTA_CURRENT, 
   D3DTA_TEMP },
 { D3DTOP_LERP, D3DTA_TEMP, D3DTA_CURRENT, D3DTA_TFACTOR | D3DTA_ALPHAREPLICATE,
   D3DTOP_SELECTARG1, D3DTA_CURRENT, D3DTA_CURRENT, D3DTA_CURRENT, 
   D3DTA_CURRENT }}
},
};

void D3DState_BuildTextureReferences()
{
	D3DState.TextureState.textureReference = 0;
	GLenum lasttexture = GL_TEXTURE0_ARB + (unsigned int)D3DGlobal.maxActiveTMU;

	for (int i = 0; i < D3DGlobal.maxActiveTMU; ++i) {
		if (!D3DState.EnableState.textureEnabled[i])
			continue;
		if ( D3DState.TextureState.TextureCombineState[i].envMode != GL_COMBINE_ARB )
			continue;

		GLenum check = D3DState.TextureState.TextureCombineState[i].colorArg1;
		if ( check == GL_TEXTURE )
			D3DState.TextureState.textureReference |= (1<<i);
		else if ( check >= GL_TEXTURE0_ARB && check < lasttexture ) {
			D3DState.TextureState.textureReference |= (1 << (check - GL_TEXTURE0_ARB));
		}
		check = D3DState.TextureState.TextureCombineState[i].alphaArg1;
		if ( check == GL_TEXTURE )
			D3DState.TextureState.textureReference |= (1<<i);
		else if ( check >= GL_TEXTURE0_ARB && check < lasttexture ) {
			D3DState.TextureState.textureReference |= (1 << (check - GL_TEXTURE0_ARB));
		}
		check = D3DState.TextureState.TextureCombineState[i].colorArg2;
		if ( check == GL_TEXTURE )
			D3DState.TextureState.textureReference |= (1<<i);
		else if ( check >= GL_TEXTURE0_ARB && check < lasttexture ) {
			D3DState.TextureState.textureReference |= (1 << (check - GL_TEXTURE0_ARB));
		}
		check = D3DState.TextureState.TextureCombineState[i].alphaArg2;
		if ( check == GL_TEXTURE )
			D3DState.TextureState.textureReference |= (1<<i);
		else if ( check >= GL_TEXTURE0_ARB && check < lasttexture ) {
			D3DState.TextureState.textureReference |= (1 << (check - GL_TEXTURE0_ARB));
		}
		if ( D3DState.TextureState.TextureCombineState[i].colorOp == GL_INTERPOLATE_ARB ) {
			check = D3DState.TextureState.TextureCombineState[i].colorArg3;
			if ( check == GL_TEXTURE )
				D3DState.TextureState.textureReference |= (1<<i);
			else if ( check >= GL_TEXTURE0_ARB && check < lasttexture ) {
				D3DState.TextureState.textureReference |= (1 << (check - GL_TEXTURE0_ARB));
			}
		}
		if ( D3DState.TextureState.TextureCombineState[i].alphaOp == GL_INTERPOLATE_ARB ) {
			check = D3DState.TextureState.TextureCombineState[i].alphaArg3;
			if ( check == GL_TEXTURE )
				D3DState.TextureState.textureReference |= (1<<i);
			else if ( check >= GL_TEXTURE0_ARB && check < lasttexture ) {
				D3DState.TextureState.textureReference |= (1 << (check - GL_TEXTURE0_ARB));
			}
		}
	}
}

bool D3DState_ValidateCombiner( int combiner, int sampler )
{
	if ( D3DState.TextureState.TextureCombineState[combiner].envMode != GL_COMBINE_ARB )
		return true;

	//invalid combiner: textureN on unit textureM
	//invalid combiner: texture as arg1 or arg2
	if ( D3DState.TextureState.TextureCombineState[combiner].colorArg1 >= GL_TEXTURE0_ARB &&
		 D3DState.TextureState.TextureCombineState[combiner].colorArg1 < (GL_TEXTURE0_ARB + (unsigned int)D3DGlobal.maxActiveTMU) ) {
		if ( D3DState.TextureState.TextureCombineState[combiner].colorArg1 != static_cast<GLenum>( GL_TEXTURE0_ARB + sampler ) ) {
			return false;
		}
	}
	if ( D3DState.TextureState.TextureCombineState[combiner].alphaArg1 >= GL_TEXTURE0_ARB &&
		 D3DState.TextureState.TextureCombineState[combiner].alphaArg1 < static_cast<GLenum>( GL_TEXTURE0_ARB + D3DGlobal.maxActiveTMU ) ) {
		if ( D3DState.TextureState.TextureCombineState[combiner].alphaArg1 != static_cast<GLenum>( GL_TEXTURE0_ARB + sampler ) ) {
			return false;
		}
	}

	if ( (D3DState.TextureState.TextureCombineState[combiner].colorArg2 == GL_TEXTURE) || 
		 (D3DState.TextureState.TextureCombineState[combiner].colorArg2 >= GL_TEXTURE0_ARB &&
		  D3DState.TextureState.TextureCombineState[combiner].colorArg2 < static_cast<GLenum>( GL_TEXTURE0_ARB + D3DGlobal.maxActiveTMU ) ) ) {
		return false;
	}
	if ( (D3DState.TextureState.TextureCombineState[combiner].alphaArg2 == GL_TEXTURE) || 
		 (D3DState.TextureState.TextureCombineState[combiner].alphaArg2 >= GL_TEXTURE0_ARB &&
		  D3DState.TextureState.TextureCombineState[combiner].alphaArg2 < static_cast<GLenum>( GL_TEXTURE0_ARB + D3DGlobal.maxActiveTMU ) ) ) {
		return false;
	}

	if ( D3DState.TextureState.TextureCombineState[combiner].colorOp == GL_INTERPOLATE_ARB ) {
		if ( (D3DState.TextureState.TextureCombineState[combiner].colorArg3 == GL_TEXTURE) || 
			 (D3DState.TextureState.TextureCombineState[combiner].colorArg3 >= GL_TEXTURE0_ARB &&
			  D3DState.TextureState.TextureCombineState[combiner].colorArg3 < static_cast<GLenum>( GL_TEXTURE0_ARB + D3DGlobal.maxActiveTMU ) ) ) {
			return false;
		}
	}
	if ( D3DState.TextureState.TextureCombineState[combiner].alphaOp == GL_INTERPOLATE_ARB ) {
		if ( (D3DState.TextureState.TextureCombineState[combiner].alphaArg3 == GL_TEXTURE) || 
			 (D3DState.TextureState.TextureCombineState[combiner].alphaArg3 >= GL_TEXTURE0_ARB &&
			  D3DState.TextureState.TextureCombineState[combiner].alphaArg3 < static_cast<GLenum>( GL_TEXTURE0_ARB + D3DGlobal.maxActiveTMU ) ) ) {
			return false;
		}
	}

	return true;
}

static const char* CombineOpToString( GLenum func )
{
	switch (func) {
	default: return "???";
	case GL_REPLACE: return "GL_REPLACE";
	case GL_MODULATE: return "GL_MODULATE";
	case GL_ADD: return "GL_ADD";
	case GL_ADD_SIGNED_ARB: return "GL_ADD_SIGNED_ARB";
	case GL_SUBTRACT_ARB: return "GL_SUBTRACT_ARB";
	case GL_INTERPOLATE_ARB: return "GL_INTERPOLATE_ARB";
	case GL_DOT3_RGB_ARB: return "GL_DOT3_RGB_ARB";
	case GL_DOT3_RGBA_ARB: return "GL_DOT3_RGBA_ARB";
	case GL_DOT3_RGB_EXT: return "GL_DOT3_RGB_EXT";
	case GL_DOT3_RGBA_EXT: return "GL_DOT3_RGBA_EXT";
	}
}

static const char* CombineArgToString( GLenum func )
{
	switch (func) {
	default: return "???";
	case GL_NONE: return "GL_NONE";
	case GL_PREVIOUS_ARB: return "GL_PREVIOUS_ARB";
	case GL_TEXTURE: return "GL_TEXTURE";
	case GL_TEXTURE0_ARB: return "GL_TEXTURE0_ARB";
	case GL_TEXTURE1_ARB: return "GL_TEXTURE1_ARB";
	case GL_TEXTURE2_ARB: return "GL_TEXTURE2_ARB";
	case GL_TEXTURE3_ARB: return "GL_TEXTURE3_ARB";
	case GL_TEXTURE4_ARB: return "GL_TEXTURE4_ARB";
	case GL_TEXTURE5_ARB: return "GL_TEXTURE5_ARB";
	case GL_TEXTURE6_ARB: return "GL_TEXTURE6_ARB";
	case GL_TEXTURE7_ARB: return "GL_TEXTURE7_ARB";
	case GL_PRIMARY_COLOR_ARB: return "GL_PRIMARY_COLOR_ARB";
	case GL_CONSTANT_ARB: return "GL_CONSTANT_ARB";
	}
}

static const char* CombineOperandToString( GLenum func )
{
	switch (func) {
	default: return "???";
	case GL_SRC_ALPHA: return "GL_SRC_ALPHA";
	case GL_ONE_MINUS_SRC_ALPHA: return "GL_ONE_MINUS_SRC_ALPHA";
	case GL_SRC_COLOR: return "GL_SRC_COLOR";
	case GL_ONE_MINUS_SRC_COLOR: return "GL_ONE_MINUS_SRC_COLOR";
	}
}

static void DumpCombiners() 
{
	int numActiveTextures = 0;
	for (int i = 0; i < D3DGlobal.maxActiveTMU; ++i) {
		if (!D3DState.EnableState.textureEnabled[i])
			continue;
		++numActiveTextures;
	}

	logPrintf("=== INVALID COMBINERS (%i TMU, %i samplers) ===\n", numActiveTextures, D3DState.TextureState.currentSamplerCount);
	for (int i = 0; i < D3DGlobal.maxActiveTMU; ++i) {
		if (!D3DState.EnableState.textureEnabled[i])
			continue;
		logPrintf(" COMBINER %i\n", i);
		logPrintf("  Color: %s %s[%s] %s[%s] %s[%s]\n", 
			CombineOpToString(D3DState.TextureState.TextureCombineState[i].colorOp),
			CombineArgToString(D3DState.TextureState.TextureCombineState[i].colorArg1),
			CombineOperandToString(D3DState.TextureState.TextureCombineState[i].colorOperand1),
			CombineArgToString(D3DState.TextureState.TextureCombineState[i].colorArg2),
			CombineOperandToString(D3DState.TextureState.TextureCombineState[i].colorOperand2),
			CombineArgToString(D3DState.TextureState.TextureCombineState[i].colorArg3),
			CombineOperandToString(D3DState.TextureState.TextureCombineState[i].colorOperand3));
		logPrintf("  Alpha: %s %s[%s] %s[%s] %s[%s]\n", 
			CombineOpToString(D3DState.TextureState.TextureCombineState[i].alphaOp),
			CombineArgToString(D3DState.TextureState.TextureCombineState[i].alphaArg1),
			CombineOperandToString(D3DState.TextureState.TextureCombineState[i].alphaOperand1),
			CombineArgToString(D3DState.TextureState.TextureCombineState[i].alphaArg2),
			CombineOperandToString(D3DState.TextureState.TextureCombineState[i].alphaOperand2),
			CombineArgToString(D3DState.TextureState.TextureCombineState[i].alphaArg3),
			CombineOperandToString(D3DState.TextureState.TextureCombineState[i].alphaOperand3));
		logPrintf(" RGB scale = %i\n", D3DState.TextureState.TextureCombineState[i].colorScale);
		logPrintf(" Alpha scale = %i\n", D3DState.TextureState.TextureCombineState[i].alphaScale);
		logPrintf("\n");
	}
}

static void D3DState_ApplyD3DCombiner( TNTCombinerReplacement *pRep )
{
	for ( int i = 0; i < pRep->numD3DCombiners; ++i ) {
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_COLOROP, pRep->D3DCombiner[i].colorOp );
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_COLORARG1, pRep->D3DCombiner[i].colorArg1 );
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_COLORARG2, pRep->D3DCombiner[i].colorArg2 );
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_COLORARG0, pRep->D3DCombiner[i].colorArg3 );
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, pRep->D3DCombiner[i].alphaOp );
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG1, pRep->D3DCombiner[i].alphaArg1 );
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG2, pRep->D3DCombiner[i].alphaArg2 );
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG0, pRep->D3DCombiner[i].alphaArg3 );
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_RESULTARG, pRep->D3DCombiner[i].resultArg );
	}
	D3DState.TextureState.currentCombinerCount = pRep->numD3DCombiners;
}

void D3DState_SetupCombiners()
{
	int numActiveTextures = 0;
	for (int i = 0; i < D3DGlobal.maxActiveTMU; ++i) {
		if (!D3DState.EnableState.textureEnabled[i])
			continue;
		++numActiveTextures;
	}

	int numReplacements = sizeof(s_TNTCombinerReplacements)/sizeof(s_TNTCombinerReplacements[0]);
	for ( int i = 0; i < numReplacements; ++i ) {
		TNTCombinerReplacement *pRep = &s_TNTCombinerReplacements[i];
		if ( numActiveTextures != pRep->numTNTCombiners )
			continue;
		bool bCmp = true;
		for ( int j = 0; j < numActiveTextures; ++j ) {
			if ( memcmp( &D3DState.TextureState.TextureCombineState[j], &pRep->TNTCombiner[j], sizeof(pRep->TNTCombiner[j]) ) ) {
				bCmp = false;
				break;
			}
		}
		if (!bCmp)
			continue;

		//found a valid replacement
		D3DState_ApplyD3DCombiner( pRep );
		return;
	}

	DumpCombiners();
}