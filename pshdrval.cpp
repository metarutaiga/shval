///////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation, 2000.
//
// pshdrval.cpp
//
// Direct3D Reference Device - PixelShader validation
//
///////////////////////////////////////////////////////////////////////////////
#include "pch.cpp"

// Use these macros when looking at CPSInstruction derived members of the current instruction (CBaseInstruction)
#define _CURR_PS_INST   ((CPSInstruction*)m_pCurrInst)
#define _PREV_PS_INST   (m_pCurrInst?((CPSInstruction*)(m_pCurrInst->m_pPrevInst)):NULL)

//-----------------------------------------------------------------------------
// CPSInstruction::CalculateComponentReadMasks()
//-----------------------------------------------------------------------------
void CPSInstruction::CalculateComponentReadMasks()
{
    BOOL bR, bG, bB, bA;

    // The only instruction that has source registers but no destination register is the 
    // XFC instruction.  If this is the XFC instruction, and the source swizzle is identity,
    // assume the read mask is rgb for all sources except G which is b.
    for( UINT i = 0; i < m_SrcParamCount; i++ )
    {
        bR=FALSE, bG=FALSE; bB=FALSE; bA=FALSE;
        if(m_DstParam[0].m_bParamUsed)
        {
            // destination is used
            if(m_DstParam[0].m_WriteMask & D3DSP_WRITEMASK_0) bR = TRUE;
            if(m_DstParam[0].m_WriteMask & D3DSP_WRITEMASK_1) bG = TRUE;
            if(m_DstParam[0].m_WriteMask & D3DSP_WRITEMASK_2) bB = TRUE;
            if(m_DstParam[0].m_WriteMask & D3DSP_WRITEMASK_3) bA = TRUE;
        }
        else
        {
            // first six xfc instruction read .rgb by default, last one reads just .b
            if(i <= 5)
                bR = bG = bB = TRUE;
            else
                bB = TRUE;
        }
        
        // DP3 instruction does not read alpha
        if(D3DSIO_DP3 == m_Type)
            bA = FALSE;

        BOOL read[4] = {FALSE, FALSE, FALSE, FALSE};
        if(bR)
            read[(m_SrcParam[i].m_SwizzleShift >> (D3DSP_SWIZZLE_SHIFT + 0)) & 0x3] = TRUE;
        if(bG)
            read[(m_SrcParam[i].m_SwizzleShift >> (D3DSP_SWIZZLE_SHIFT + 2)) & 0x3] = TRUE;
        if(bB)
            read[(m_SrcParam[i].m_SwizzleShift >> (D3DSP_SWIZZLE_SHIFT + 4)) & 0x3] = TRUE;
        if(bA)
            read[(m_SrcParam[i].m_SwizzleShift >> (D3DSP_SWIZZLE_SHIFT + 6)) & 0x3] = TRUE;
        
        m_SrcParam[i].m_ComponentReadMask = 0;
        if(read[0])
            m_SrcParam[i].m_ComponentReadMask |= D3DSP_WRITEMASK_0;
        if(read[1])
            m_SrcParam[i].m_ComponentReadMask |= D3DSP_WRITEMASK_1;
        if(read[2])
            m_SrcParam[i].m_ComponentReadMask |= D3DSP_WRITEMASK_2;
        if(read[3])
            m_SrcParam[i].m_ComponentReadMask |= D3DSP_WRITEMASK_3;
    }
}

//-----------------------------------------------------------------------------
// CPShaderValidator::CPShaderValidator
//-----------------------------------------------------------------------------
CPShaderValidator::CPShaderValidator(   const DWORD* pCode,
                                        const D3DCAPS8* pCaps,
                                        DWORD Flags)
                                        : CBaseShaderValidator( pCode, pCaps, Flags )
{
    // Note that the base constructor initialized m_ReturnCode to E_FAIL.
    // Only set m_ReturnCode to S_OK if validation has succeeded,
    // before exiting this constructor.

    m_TexOpCount            = 0;
    m_BlendOpCount          = 0;
    m_TotalOpCount          = 0;
    m_TexMBaseDstReg        = 0;

    m_pTempRegFile          = NULL;
    m_pInputRegFile         = NULL;
    m_pConstRegFile         = NULL;
    m_pTextureRegFile       = NULL;

    if( !m_bBaseInitOk )
        return;

    ValidateShader(); // If successful, m_ReturnCode will be set to S_OK.
                      // Call GetStatus() on this object to determine validation outcome.
}

//-----------------------------------------------------------------------------
// CPShaderValidator::~CPShaderValidator
//-----------------------------------------------------------------------------
CPShaderValidator::~CPShaderValidator()
{
    delete m_pTempRegFile;
    delete m_pInputRegFile;
    delete m_pConstRegFile;
    delete m_pTextureRegFile;
}

//-----------------------------------------------------------------------------
// CPShaderValidator::AllocateNewInstruction
//-----------------------------------------------------------------------------
CBaseInstruction* CPShaderValidator::AllocateNewInstruction(CBaseInstruction*pPrevInst)
{
    return new CPSInstruction((CPSInstruction*)pPrevInst);
}

//-----------------------------------------------------------------------------
// CPShaderValidator::DecodeNextInstruction
//-----------------------------------------------------------------------------
BOOL CPShaderValidator::DecodeNextInstruction()
{
    m_pCurrInst->m_Type = (D3DSHADER_INSTRUCTION_OPCODE_TYPE)(*m_pCurrToken & D3DSI_OPCODE_MASK);

    if( m_pCurrInst->m_Type == D3DSIO_COMMENT )
    {
        ParseCommentForAssemblerMessages(m_pCurrToken); // does not advance m_pCurrToken

        // Skip comments
        DWORD NumDWORDs = ((*m_pCurrToken) & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
        m_pCurrToken += (NumDWORDs+1);
        return TRUE;
    }

    // If the assembler has sent us file and/or line number messages,
    // received by ParseCommentForAssemblerMesssages(),
    // then bind this information to the current instruction.
    // This info can be used in error spew to direct the shader developer
    // to exactly where a problem is located.
    m_pCurrInst->SetSpewFileNameAndLineNumber(m_pLatestSpewFileName,m_pLatestSpewLineNumber);

    if( *m_pCurrToken & D3DSI_COISSUE )
    {
        _CURR_PS_INST->m_bCoIssue = TRUE;
    }
    else
    {
        m_CycleNum++; // First cycle is 1. (co-issued instructions will have same cycle number)
    }
    _CURR_PS_INST->m_CycleNum = m_CycleNum;

    m_SpewInstructionCount++; // only used for spew, not for any limits
    m_pCurrInst->m_SpewInstructionCount = m_SpewInstructionCount;

    if( (*m_pCurrToken) & PS_INST_TOKEN_RESERVED_MASK )
    {
        Spew(SPEW_INSTRUCTION_ERROR,m_pCurrInst,"Reserved bit(s) set in instruction parameter token!  Aborting validation.");
        return FALSE;
    }

    m_pCurrToken++;

    // Decode dst param
    DWORD dstCount;
    switch (m_pCurrInst->m_Type)
    {
    case D3DSIO_NOP:
        dstCount = 0;
        break;
    default:
        dstCount = 1;
        break;
    }
    while ((*m_pCurrToken & (1L<<31)) && (dstCount-- > 0))
    {
        (m_pCurrInst->m_DstParamCount)++;
        DecodeDstParam( m_pCurrInst->m_Type, &m_pCurrInst->m_DstParam[m_pCurrInst->m_DstParamCount - 1], *m_pCurrToken );
        if( (*m_pCurrToken) & PS_DSTPARAM_TOKEN_RESERVED_MASK )
        {
            Spew(SPEW_INSTRUCTION_ERROR,m_pCurrInst,"Reserved bit(s) set in destination parameter token!  Aborting validation.");
            return FALSE;
        }
        m_pCurrToken++;
        if( D3DSIO_DEF == m_pCurrInst->m_Type )
        {
            // Skip source params (float vector) - nothing to check
            // This is the only instruction with 4 source params,
            // and further, this is the only instruction that has
            // raw numbers as parameters.  This justifies the
            // special case treatment here - we pretend
            // D3DSIO_DEF only has a dst param (which we will check).
            m_pCurrToken += 4;
            return TRUE;
        }
    }

    // Decode src param(s)
    while (*m_pCurrToken & (1L<<31))
    {
        (m_pCurrInst->m_SrcParamCount)++;
        if( (m_pCurrInst->m_SrcParamCount + m_pCurrInst->m_DstParamCount) > SHADER_INSTRUCTION_MAX_PARAMS )
        {
            m_pCurrToken++; // eat up extra parameters and skip to next
            continue;
        }

        // Below: index is [SrcParamCount - 1] because m_SrcParam array needs 0 based index.
        DecodeSrcParam( &(m_pCurrInst->m_SrcParam[m_pCurrInst->m_SrcParamCount - 1]),*m_pCurrToken );

        if( (*m_pCurrToken) & PS_SRCPARAM_TOKEN_RESERVED_MASK )
        {
            Spew(SPEW_INSTRUCTION_ERROR,m_pCurrInst,"Reserved bit(s) set in source %d parameter token!  Aborting validation.",
                            m_pCurrInst->m_SrcParamCount);
            return FALSE;
        }
        m_pCurrToken++;
    }

    // Figure out which components of each source operand actually need to be read,
    // taking into account destination write mask, the type of instruction, source swizzle, etc.
    m_pCurrInst->CalculateComponentReadMasks();

    // Find out if the instruction is a TexOp and/or TexMOp.  Needed by multiple validation rules.
    IsCurrInstTexOp();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// CPShaderValidator Wrapper Functions
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ValidatePixelShaderInternal
//-----------------------------------------------------------------------------
BOOL ValidatePixelShaderInternal( const DWORD* pCode, const D3DCAPS8* pCaps )
{
    CPShaderValidator Validator(pCode,pCaps,0);
    return SUCCEEDED(Validator.GetStatus()) ? TRUE : FALSE;
}

//-----------------------------------------------------------------------------
// ValidatePixelShader
//
//-----------------------------------------------------------------------------
HRESULT WINAPI ValidatePixelShader( const DWORD* pCode,
                                    const D3DCAPS8* pCaps,
                                    const DWORD Flags )
{
    CPShaderValidator Validator(pCode,pCaps,Flags);
#if 0
    if( ppBuf )
    {
        *ppBuf = (char*)HeapAlloc(GetProcessHeap(), 0, Validator.GetRequiredLogBufferSize());
        if( NULL == *ppBuf )
            OutputDebugString("Out of memory.\n");
        else
            Validator.WriteLogToBuffer(*ppBuf);
    }
#endif
    return Validator.GetStatus();
}
