/*==========================================================================
 *
 *  Copyright (C) 1999 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       vvm.cpp
 *  Content:    Virtual Vertex Machine implementation
 *
 *
 ***************************************************************************/
#include "pch.cpp"
#pragma hdrstop

#if DBG
//-----------------------------------------------------------------------------
// VertexShaderInstDisAsm - Generates human-readable character string for a
// single vertex shader instruction.  String interface is similar to _snprintf.
//-----------------------------------------------------------------------------
int VertexShaderInstDisAsm(
    char* pStrRet, int StrSizeRet, DWORD* pShader, DWORD Flags )
{
    DWORD*  pToken = pShader;

    // stage in local string, then copy
    char pStr[256] = "";
#define _ADDSTR( _Str ) { _snprintf( pStr, 256, "%s" _Str , pStr ); }
#define _ADDSTRP( _Str, _Param ) { _snprintf( pStr, 256, "%s" _Str , pStr, _Param ); }

    DWORD Inst = *pToken++;
    DWORD Opcode = (Inst & D3DSI_OPCODE_MASK);
    switch (Opcode)
    {
    case D3DSIO_NOP:  _ADDSTR("NOP");  break;
    case D3DSIO_MOV:  _ADDSTR("MOV");  break;
    case D3DSIO_ADD:  _ADDSTR("ADD");  break;
    case D3DSIO_MAD:  _ADDSTR("MAD");  break;
    case D3DSIO_MUL:  _ADDSTR("MUL");  break;
    case D3DSIO_RCP:  _ADDSTR("RCP");  break;
    case D3DSIO_RSQ:  _ADDSTR("RSQ");  break;
    case D3DSIO_DP3:  _ADDSTR("DP3");  break;
    case D3DSIO_DP4:  _ADDSTR("DP4");  break;
    case D3DSIO_MIN:  _ADDSTR("MIN");  break;
    case D3DSIO_MAX:  _ADDSTR("MAX");  break;
    case D3DSIO_SLT:  _ADDSTR("SLT");  break;
    case D3DSIO_SGE:  _ADDSTR("SGE");  break;
    case D3DSIO_EXP:  _ADDSTR("EXP");  break;
    case D3DSIO_LOG:  _ADDSTR("LOG");  break;
    case D3DSIO_EXPP: _ADDSTR("EXPP"); break;
    case D3DSIO_LOGP: _ADDSTR("LOGP"); break;
    case D3DSIO_LIT:  _ADDSTR("LIT");  break;
    case D3DSIO_DST:  _ADDSTR("DST");  break;
    default        :  _ADDSTR("???");  break;
    }
    if (*pToken & (1L<<31))
    {
        DWORD DstParam = *pToken++;
        switch (DstParam & D3DSP_REGTYPE_MASK)
        {
        case D3DSPR_TEMP     : _ADDSTRP(" T%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_ADDR     : _ADDSTR(" Addr"); break;
        case D3DSPR_RASTOUT  : _ADDSTRP(" R%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_ATTROUT  : _ADDSTRP(" A%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_TEXCRDOUT: _ADDSTRP(" T%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        }
        if (*pToken & (1L<<31)) _ADDSTR(" ");
        while (*pToken & (1L<<31))
        {
            DWORD SrcParam = *pToken++;
            switch (SrcParam & D3DSP_REGTYPE_MASK)
            {
            case D3DSPR_TEMP     : _ADDSTRP(" T%d", (SrcParam & D3DSP_REGNUM_MASK) ); break;
            case D3DSPR_INPUT    : _ADDSTRP(" I%d", (SrcParam & D3DSP_REGNUM_MASK) ); break;
            case D3DSPR_CONST    : _ADDSTRP(" C%d", (SrcParam & D3DSP_REGNUM_MASK) ); break;
            }
            if (*pToken & (1L<<31)) _ADDSTR(",");
        }
    }
    return _snprintf( pStrRet, StrSizeRet, "%s", pStr );
}
#endif // DBG
