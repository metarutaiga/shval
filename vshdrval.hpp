///////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation, 2000.
//
// vshdrval.hpp
//
// Direct3D Reference Device - VertexShader validation
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __VSHDRVAL_HPP__
#define __VSHDRVAL_HPP__

//-----------------------------------------------------------------------------
// CVSInstruction
//-----------------------------------------------------------------------------
class CVSInstruction : public CBaseInstruction
{
public:
    CVSInstruction(CVSInstruction* pPrevInst) : CBaseInstruction(pPrevInst) {};

    void CalculateComponentReadMasks();
};

//-----------------------------------------------------------------------------
// CVShaderValidator
//-----------------------------------------------------------------------------
class CVShaderValidator : public CBaseShaderValidator
{
private:
    void ValidateDeclaration();
    const DWORD*    m_pDecl;
    BOOL            m_bFixedFunction;

    CRegisterFile*  m_pTempRegFile;    
    CRegisterFile*  m_pInputRegFile;
    CRegisterFile*  m_pConstRegFile;
    CRegisterFile*  m_pAddrRegFile;
    CRegisterFile*  m_pTexCrdOutputRegFile;
    CRegisterFile*  m_pAttrOutputRegFile;
    CRegisterFile*  m_pRastOutputRegFile;

    CBaseInstruction* AllocateNewInstruction(CBaseInstruction*pPrevInst);
    BOOL DecodeNextInstruction();
    BOOL InitValidation();
    BOOL ApplyPerInstructionRules();
    void ApplyPostInstructionsRules();

    BOOL Rule_InstructionRecognized();
    BOOL Rule_InstructionSupportedByVersion();
    BOOL Rule_ValidParamCount();
    BOOL Rule_ValidSrcParams(); 
    BOOL Rule_SrcInitialized();
    BOOL Rule_ValidAddressRegWrite();
    BOOL Rule_ValidDstParam();
    BOOL Rule_ValidFRCInstruction();
    BOOL Rule_ValidRegisterPortUsage();
    BOOL Rule_ValidInstructionCount();             // Call per instruction AND after all instructions seen
    BOOL Rule_oPosWritten();                       // Call after all instructions seen

    DWORD GetMinRegNumber(DWORD type);
    
    void  DecodeDstParam(D3DSHADER_INSTRUCTION_OPCODE_TYPE inst, DSTPARAM* pDstParam, DWORD Token ); // replace method on base class
    void  DecodeSrcParam( SRCPARAM* pSrcParam, DWORD Token ); // replace method on base class

public:
    CVShaderValidator(  const DWORD* pCode, 
                        const DWORD* pDecl, 
                        const D3DCAPS8* pCaps, 
                        DWORD Flags );
    ~CVShaderValidator();
};

HRESULT WINAPI ValidateVertexShader(    const DWORD* pCode, 
                                        const DWORD* pDecl,
                                        const D3DCAPS8* pCaps, 
                                        const DWORD Flags );

#endif __VSHDRVAL_HPP__
