///////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation, 2000.
//
// pshader.h
//
// Direct3D Reference Device - Pixel Shader
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _PSHADER_H
#define _PSHADER_H

// refdev-specific pixel shader 'instructions' to match legacy pixel processing
#define D3DSIO_TEXBEM_LEGACY    ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC001)
#define D3DSIO_TEXBEML_LEGACY   ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC002)

// psutil.cpp
int
PixelShaderInstDisAsm(
    char* pStrRet, int StrSizeRet, DWORD* pShader, DWORD Flags );

#endif _PSHADER_H
