// =======================================================================
// File: conv_cdecl.cpp
// Purpose: Defines the implementation of the CCdecl_Convention class.
//	This class will generate ASM code for saving registers and 
//	cleaning up the stack.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "conv_cdecl.h"

// =======================================================================
// Namespaces to use.
// =======================================================================
using namespace AsmJit;

// =======================================================================
// Stage 1. Called by the detour system to save registers.
// =======================================================================
void CCdecl_Convention::Stage_1( Assembler* pAssembler )
{
	// ------------------------------------
	// We need to save ESP, and that's it!
	// ------------------------------------
	pAssembler->mov( dword_ptr_abs(&m_Registers.r_esp), esp );
	
}

// =======================================================================
// Stage 2. Called by the detour system to restore registers.
// =======================================================================
void CCdecl_Convention::Stage_2( Assembler* pAssembler )
{
	// ------------------------------------
	// Restore ESP.
	// ------------------------------------
	pAssembler->mov( esp, dword_ptr_abs(&m_Registers.r_esp) );
}

// =======================================================================
// Stage 3. Called by the detour system to generate code before overriding
//	the original return value.
// =======================================================================
void CCdecl_Convention::Stage_3( AsmJit::Assembler* pAssembler )
{
	return;
}
