// =======================================================================
// File: asmbridge_class.h
// Purpose: Implementation of CASMBridge.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "hook_handler.h"
#include "hook_types.h"
#include "conv_main.h"
#include "asmbridge_class.h"
#include "detour_class.h"
#include "func_class.h"

// =======================================================================
// Namespaces to use.
// =======================================================================
using namespace AsmJit;

// =======================================================================
// Constructor
// =======================================================================
CASMBridge::CASMBridge( CDetour* pDetour )
{
	// ------------------------------------
	// If any of these are null, we can't
	// continue.
	// ------------------------------------
	if( !pDetour || !pDetour->GetFuncObj() )
	{
		m_bInitialized = false;
		return;
	}

	// ------------------------------------
	// Get the function object.
	// ------------------------------------
	CFuncObj* pFuncObj = pDetour->GetFuncObj();

	// ------------------------------------
	// Create the calling convention
	// instance.
	// ------------------------------------
	m_pCallConvention = 
		EnumToConvention(pFuncObj->GetConvention());

	// ------------------------------------
	// If this is not valid, we can't
	// continue.
	// ------------------------------------
	if( !m_pCallConvention )
	{
		m_bInitialized = false;
		return;
	}

	// ------------------------------------
	// Initialize stage 1.
	// ------------------------------------
	m_pCallConvention->Stage_1( &m_Assembler );

	// ------------------------------------
	// Now we need to add code to call the
	// callback.
	// ------------------------------------
	// 1) push detour
	// 2) call function
	// 3) add 0x4 to remove detour from stack.
	// 4) If the result is
	// ------------------------------------
	m_Assembler.push( imm((SysInt)pDetour) );
	m_Assembler.call( (void*)&Dyn_PreHandler );
	m_Assembler.add( esp, imm(4) );

	// ------------------------------------
	// Figure out what to do with the
	// result.
	// ------------------------------------
	m_Assembler.cmp( eax, HOOKRES_OVERRIDE );
	m_Assembler.je( &m_Override );
	m_Assembler.jmp( &m_PostCall );

	// ------------------------------------
	// This is the post call label.
	// It contains instructions to restore
	// ESP and call the original function.
	// ------------------------------------

	// ------------------------------------
	// Bind us to the postcall label.
	// ------------------------------------
	m_Assembler.bind( &m_PostCall );

	// ------------------------------------
	// Call stage_2. This is basically
	// register cleanup code.
	// ------------------------------------
	m_pCallConvention->Stage_2( &m_Assembler );

	// ------------------------------------
	// Post calls still call the original
	// function.
	// ------------------------------------
	m_Assembler.jmp( pDetour->GetTrampoline() );

	// ------------------------------------
	// Return (for now). We would call
	// post-execution callbacks here..
	// ------------------------------------
	m_Assembler.ret();

	// ------------------------------------
	// This code is for overriding
	// the original function.
	// ------------------------------------
	CRegisterObj* registers = m_pCallConvention->GetRegisters();
	
	// ------------------------------------
	// Bind the override label to this
	// location.
	// ------------------------------------
	m_Assembler.bind( &m_Override );

	// ------------------------------------
	// Generate the code.
	// ------------------------------------
	m_pCallConvention->Stage_3( &m_Assembler );

	// ------------------------------------
	// Use the new the return value.
	// ------------------------------------
	m_Assembler.mov( eax, dword_ptr_abs(&registers->r_eax) );
	
	// ------------------------------------
	// Hack for __thiscall.
	// ------------------------------------
	if( (pFuncObj->GetConvention() == CONV_THISCALL) || 
		(pFuncObj->GetConvention() == CONV_STDCALL) )
	{
		// ------------------------------------
		// Get the size in bytes of the stack.
		// ------------------------------------
		int iBytesToClean = pFuncObj->GetStack()->GetStackSize();
		m_Assembler.ret( imm(iBytesToClean) );
	}

	// ------------------------------------
	// Just return.
	// ------------------------------------
	m_Assembler.ret();

	// ------------------------------------
	// We are now done initializing.
	// ------------------------------------
	m_bInitialized = true;
}

// =======================================================================
// Destructor
// =======================================================================
CASMBridge::~CASMBridge( void )
{
	delete m_pCallConvention;
	m_Assembler.free();
}
