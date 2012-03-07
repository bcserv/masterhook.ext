// =======================================================================
// File: conv_stdcall.cpp
// Purpose: Defines the implementation of the CStdCall_Convention class.
//	This class will generate ASM code for saving registers and 
//	cleaning up the stack.
//
// From Wikipedia:
//	The stdcall calling convention is a variation on the pascal calling 
//	convention in which the callee is responsible to cleanup that stack, 
//	but the parameter are passed on the stack, pushed right-to-left 
// (similar to _cdecl calling convention). Registers EAX, ECX, and EDX 
//	are designated for use within the function. Return values are stored 
//	in the EAX register.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "conv_stdcall.h"

// =======================================================================
// Namespaces to use.
// =======================================================================
using namespace AsmJit;

// =======================================================================
// Stage 1. Called by the detour system to save registers.
// =======================================================================
void CStdCall_Convention::Stage_1( Assembler* pAssembler )
{
	// ------------------------------------
	// Save ESP, ECX, and EDX.
	// ------------------------------------
	pAssembler->mov( dword_ptr_abs(&m_Registers.r_esp), esp );
	pAssembler->mov( dword_ptr_abs(&m_Registers.r_ecx), ecx );
	pAssembler->mov( dword_ptr_abs(&m_Registers.r_edx), edx );
}

// =======================================================================
// Stage 2. Called by the detour system to restore registers.
// =======================================================================
void CStdCall_Convention::Stage_2( Assembler* pAssembler )
{
	// ------------------------------------
	// Restore ESP.
	// ------------------------------------
	pAssembler->mov( esp, dword_ptr_abs(&m_Registers.r_esp) );
	pAssembler->mov( ecx, dword_ptr_abs(&m_Registers.r_ecx) );
	pAssembler->mov( edx, dword_ptr_abs(&m_Registers.r_edx) );
}

// =======================================================================
// Stage 3. Called by the detour system to generate code before overriding
//	the original return value.
// =======================================================================
void CStdCall_Convention::Stage_3( AsmJit::Assembler* pAssembler )
{
	return;
}
