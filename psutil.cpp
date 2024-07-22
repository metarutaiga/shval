///////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation, 2000.
//
// psutil.cpp
//
// Direct3D Reference Device - Pixel Shader Utilities
//
///////////////////////////////////////////////////////////////////////////////
#include "pch.cpp"
#pragma hdrstop

#if DBG
#define _ADDSTR( _Str )             {_snprintf( pStr, 256, "%s" _Str , pStr );}
#define _ADDSTRP( _Str, _Param )    {_snprintf( pStr, 256, "%s" _Str , pStr, _Param );}

//-----------------------------------------------------------------------------
//
// PixelShaderInstDisAsm - Generates instruction disassembly string for a single
// pixel shader instruction.  String interface is similar to _snprintf.
//
//-----------------------------------------------------------------------------
int
PixelShaderInstDisAsm(
    char* pStrRet, int StrSizeRet, DWORD* pShader, DWORD Flags )
{
    UINT    i,j;
    DWORD*  pToken = pShader;
    
    // stage in local string, then copy
    char pStr[256] = "";

    DWORD Inst = *pToken++;

    if( Inst & D3DSI_COISSUE )
    {
        _ADDSTR("+");
    }

    DWORD Opcode = (Inst & D3DSI_OPCODE_MASK);
    DWORD DstParam = 0;
    DWORD SrcParam[3];
    DWORD SrcParamCount = 0;

    if (*pToken & (1L<<31))
    {
        DstParam = *pToken++;
        while (*pToken & (1L<<31))
        {
            SrcParam[SrcParamCount] = *pToken++;
            SrcParamCount++;
        }
    }

    switch (Opcode)
    {
    case D3DSIO_PHASE: _ADDSTR("phase"); break;
    case D3DSIO_NOP: _ADDSTR("nop"); break;
    case D3DSIO_MOV: _ADDSTR("mov"); break;
    case D3DSIO_ADD: _ADDSTR("add"); break;
    case D3DSIO_SUB: _ADDSTR("sub"); break;
    case D3DSIO_MUL: _ADDSTR("mul"); break;
    case D3DSIO_MAD: _ADDSTR("mad"); break;
    case D3DSIO_LRP: _ADDSTR("lrp"); break;
    case D3DSIO_CND: _ADDSTR("cnd"); break;
    case D3DSIO_DP3: _ADDSTR("dp3"); break;
    case D3DSIO_DEF: _ADDSTR("def"); break;
    case D3DSIO_DP4: _ADDSTR("dp4"); break;
    case D3DSIO_CMP: _ADDSTR("cmp"); break;
    case D3DSIO_FRC: _ADDSTR("frc"); break;
    case D3DSIO_BEM: _ADDSTR("bem"); break;

    case D3DSIO_TEXCOORD    : if(SrcParamCount)
                                  _ADDSTR("texcrd")
                              else 
                                  _ADDSTR("texcoord"); 
                              break;
    case D3DSIO_TEX         : if(SrcParamCount)
                                  _ADDSTR("texld")
                              else
                                  _ADDSTR("tex"); 
                              break;
    case D3DSIO_TEXKILL     : _ADDSTR("texkill"); break;
    case D3DSIO_TEXBEM_LEGACY:
    case D3DSIO_TEXBEM      : _ADDSTR("texbem"); break;
    case D3DSIO_TEXBEML_LEGACY:
    case D3DSIO_TEXBEML     : _ADDSTR("texbeml"); break;
    case D3DSIO_TEXREG2AR   : _ADDSTR("texreg2ar"); break;
    case D3DSIO_TEXREG2GB   : _ADDSTR("texreg2gb"); break;
    case D3DSIO_TEXM3x2PAD  : _ADDSTR("texm3x2pad"); break;
    case D3DSIO_TEXM3x2TEX  : _ADDSTR("texm3x2tex"); break;
    case D3DSIO_TEXM3x3PAD  : _ADDSTR("texm3x3pad"); break;
    case D3DSIO_TEXM3x3TEX  : _ADDSTR("texm3x3tex"); break;
    case D3DSIO_TEXM3x3SPEC : _ADDSTR("texm3x3spec"); break;
    case D3DSIO_TEXM3x3VSPEC: _ADDSTR("texm3x3vspec"); break;
    case D3DSIO_TEXM3x2DEPTH : _ADDSTR("texm3x2depth"); break;
    case D3DSIO_TEXDP3      : _ADDSTR("texdp3"); break;
    case D3DSIO_TEXREG2RGB  : _ADDSTR("texreg2rgb"); break;
    case D3DSIO_TEXDEPTH    : _ADDSTR("texdepth"); break;
    case D3DSIO_TEXDP3TEX   : _ADDSTR("texdp3tex"); break;
    case D3DSIO_TEXM3x3     : _ADDSTR("texm3x3"); break;
    case D3DSIO_END         : _ADDSTR("END"); break;
    default:
        _ASSERT(FALSE,"Attempt to disassemble unknown instruction!");
    }

    if (DstParam)
    {
        switch ( (DstParam & D3DSP_DSTSHIFT_MASK) >> D3DSP_DSTSHIFT_SHIFT )
        {
        default:
        case 0x0: break;
        case 0x1: _ADDSTR("_x2"); break;
        case 0x2: _ADDSTR("_x4"); break;
        case 0x3: _ADDSTR("_x8"); break;
        case 0xF: _ADDSTR("_d2"); break;
        case 0xE: _ADDSTR("_d4"); break;
        case 0xD: _ADDSTR("_d8"); break;
        }
        switch (DstParam & D3DSP_DSTMOD_MASK)
        {
        default:
        case D3DSPDM_NONE:      break;
        case D3DSPDM_SATURATE:  _ADDSTR("_sat"); break;
        }

        switch (DstParam & D3DSP_REGTYPE_MASK)
        {
        default:
        case D3DSPR_TEMP:    _ADDSTRP(" r%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_TEXTURE: _ADDSTRP(" t%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_CONST:   _ADDSTRP(" c%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        }
        if (D3DSP_WRITEMASK_ALL != (DstParam & D3DSP_WRITEMASK_ALL))
        {
            _ADDSTR(".");
            if (DstParam & D3DSP_WRITEMASK_0) _ADDSTR("r");
            if (DstParam & D3DSP_WRITEMASK_1) _ADDSTR("g");
            if (DstParam & D3DSP_WRITEMASK_2) _ADDSTR("b");
            if (DstParam & D3DSP_WRITEMASK_3) _ADDSTR("a");
        }

        if( D3DSIO_DEF == Opcode )
        {
            for( i = 0; i < 4; i++ )
                _ADDSTRP(", %f", *(float*)&(*pToken++) );
            goto EXIT;
        }
    }

    for( i = 0; i < SrcParamCount; i++ )
    {
        _ADDSTR(",");

        switch (SrcParam[i] & D3DSP_SRCMOD_MASK)
        {
        default:
        case D3DSPSM_NONE:    _ADDSTR(" "); break;
        case D3DSPSM_NEG:     _ADDSTR(" -"); break;
        case D3DSPSM_BIAS:    _ADDSTR(" "); break;
        case D3DSPSM_BIASNEG: _ADDSTR(" -"); break;
        case D3DSPSM_SIGN:    _ADDSTR(" "); break;
        case D3DSPSM_SIGNNEG: _ADDSTR(" -"); break;
        case D3DSPSM_COMP:    _ADDSTR(" 1-"); break;
        case D3DSPSM_X2:      _ADDSTR(" "); break;
        case D3DSPSM_X2NEG:   _ADDSTR(" -"); break;
        }
        switch (SrcParam[i] & D3DSP_REGTYPE_MASK)
        {
        case D3DSPR_TEMP:    _ADDSTRP("r%d", (SrcParam[i] & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_INPUT:   _ADDSTRP("v%d", (SrcParam[i] & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_CONST:   _ADDSTRP("c%d", (SrcParam[i] & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_TEXTURE: _ADDSTRP("t%d", (SrcParam[i] & D3DSP_REGNUM_MASK) ); break;
        }
        switch (SrcParam[i] & D3DSP_SRCMOD_MASK)
        {
        default:
        case D3DSPSM_NONE:    break;
        case D3DSPSM_NEG:     break;
        case D3DSPSM_BIAS:    _ADDSTR("_bias"); break;
        case D3DSPSM_BIASNEG: _ADDSTR("_bias"); break;
        case D3DSPSM_SIGN:    _ADDSTR("_bx2"); break;
        case D3DSPSM_SIGNNEG: _ADDSTR("_bx2"); break;
        case D3DSPSM_COMP:    break;
        case D3DSPSM_X2:      _ADDSTR("_x2"); break;
        case D3DSPSM_X2NEG:   _ADDSTR("_x2"); break;
        case D3DSPSM_DZ:      _ADDSTR("_db"); break;
        case D3DSPSM_DW:      _ADDSTR("_da"); break;
        }
        switch (SrcParam[i] & D3DVS_SWIZZLE_MASK)
        {
        case D3DSP_NOSWIZZLE:       break;
        case D3DSP_REPLICATEALPHA:  _ADDSTR(".a"); break;
        case D3DSP_REPLICATERED:    _ADDSTR(".r"); break;
        case D3DSP_REPLICATEGREEN:  _ADDSTR(".g"); break;
        case D3DSP_REPLICATEBLUE:   _ADDSTR(".b"); break;
        default:
            _ADDSTR(".");
            for(j = 0; j < 4; j++)
            {
                switch(((SrcParam[i] & D3DVS_SWIZZLE_MASK) >> (D3DVS_SWIZZLE_SHIFT + 2*j)) & 0x3)
                {
                case 0:
                    _ADDSTR("r");
                    break;
                case 1:
                    _ADDSTR("g");
                    break;
                case 2:
                    _ADDSTR("b");
                    break;
                case 3:
                    _ADDSTR("a");
                    break;
                }
            }
            break;
        }
    }
EXIT:
    return _snprintf( pStrRet, StrSizeRet, "%s", pStr );
}
#endif // DBG
