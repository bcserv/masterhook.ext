// =======================================================================
// File: trampoline_class.cpp
// Purpose: Implementation of CTrampoline.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "trampoline_class.h"
#include "asm.h"
#include "memutils.h"

// =======================================================================
// Useful constants.
// =======================================================================
#define JMP_SIZE 6

// =======================================================================
// Constructor
// =======================================================================
CTrampoline::CTrampoline( void* pTarget )
{
	// ------------------------------------
	// Can't continue if the target is
	// not valid.
	// ------------------------------------
	if( !pTarget )
	{
		m_pSavedBytes = NULL;
		m_pTarget = NULL;
		m_iSavedBytes = 0;
		return;
	}

	m_pTarget = (unsigned char*)pTarget;

	// ------------------------------------
	// Figure out how many bytes we need
	// to copy from the original function.
	// 6 is used because we need to perform
	// a 6 byte jump to the ASM bridge from
	// the target function.
	// ------------------------------------
	m_iSavedBytes = copy_bytes( m_pTarget, NULL, 6 );

	// ------------------------------------
	// Now allocate the space for
	// m_iSavedBytes + 6 more. We need to
	// jump back to the target function + JMP_SIZE.
	// ------------------------------------
	m_pSavedBytes = new unsigned char[m_iSavedBytes + JMP_SIZE];

	// ------------------------------------
	// Clear out the array.
	// ------------------------------------
	memset( m_pSavedBytes, 0x90, m_iSavedBytes + JMP_SIZE );

	// ------------------------------------
	// Make sure we can execute the code
	// in the array.
	// ------------------------------------
	SetMemPatchable( m_pSavedBytes, m_iSavedBytes + JMP_SIZE );

	// ------------------------------------
	// Copy the bytes from the original
	// function to our array.
	// ------------------------------------
	copy_bytes( m_pTarget, m_pSavedBytes, JMP_SIZE );

	// ------------------------------------
	// Now write a jump to the target
	// function + savedBytes.
	// ------------------------------------
	unsigned char* src = m_pSavedBytes + m_iSavedBytes;
	unsigned char* dest = m_pTarget + m_iSavedBytes;
	WriteJMP( src, dest );
}

// =======================================================================
// Destructor.
// =======================================================================
CTrampoline::~CTrampoline( void )
{
	// ------------------------------------
	// Restore the bytes.
	// ------------------------------------
	Restore();

	// ------------------------------------
	// Now free the memory.
	// ------------------------------------
	delete m_pSavedBytes;
}

// =======================================================================
// Restores the target function's original bytes.
// =======================================================================
bool CTrampoline::Restore( void )
{
	// ------------------------------------
	// Write the saved bytes back to the
	// target. NOTE: We don't add JMP_SIZE
	// here because we don't want to copy
	// our own jump! We just want the bytes
	// we took from the original function.
	// ------------------------------------
	if( m_pSavedBytes && m_pTarget ) {
		SetMemPatchable( m_pTarget, m_iSavedBytes );
		memcpy(m_pTarget, m_pSavedBytes, m_iSavedBytes);
		return true;
	}

	// ------------------------------------
	// Something went wrong if we're here.
	// ------------------------------------
	return false;
}
