// =======================================================================
// File: func_stack.cpp
// Purpose: Provides implementations of the methods declared in
//	func_stack.h.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "func_stack.h"

#define NULL 0

// =======================================================================
// Constructor
// =======================================================================
CFuncStack::CFuncStack( void )
{
	// ------------------------------------
	// Initialize the list head node to 0.
	// ------------------------------------
	m_pArgListHead = NULL;

	// ------------------------------------
	// Initialize the number arguments to
	// -1.
	// ------------------------------------
	m_nNumArgs = 0;

	// ------------------------------------
	// Initialize the stack size to 0.
	// ------------------------------------
	m_nTotalSize = 0;
}

// =======================================================================
// Destructor.
// =======================================================================
CFuncStack::~CFuncStack( void )
{
	// ------------------------------------
	// Need to free each node.
	// ------------------------------------
	ArgNode_t* pTemp = m_pArgListHead;
	while( pTemp )
	{
		ArgNode_t* pNext = pTemp->m_pNext;
		
		delete pTemp->m_pArg;
		delete pTemp;
		
		pTemp = pNext;
	}
}

// =======================================================================
// Adds an argument to the stack.
// =======================================================================
bool CFuncStack::AddArgument( CFuncArg* pArg )
{
	// ------------------------------------
	// Make sure the argument instance is
	// valid.
	// ------------------------------------
	if( !pArg )
	{
		return false;
	}

	// ------------------------------------
	// Create the ArgNode_t instance.
	// ------------------------------------
	ArgNode_t* pNode = new ArgNode_t;

	// ------------------------------------
	// Set it up.
	// ------------------------------------
	pNode->m_pArg = pArg;
	pNode->m_pNext = NULL;

	// ------------------------------------
	// The current offset of the arg from
	// esp is whatever the current total
	// size is.
	// ------------------------------------
	pNode->m_nOffset = m_nTotalSize;

	// ------------------------------------
	// We increment the total size now by
	// the total size of the argument
	// we are adding. This gets us ready
	// for the next argument.
	// ------------------------------------
	m_nTotalSize += pArg->GetSize();

	// ------------------------------------
	// If the arglist is NULL, it means
	// we haven't initialized it yet.
	// ------------------------------------
	if( !m_pArgListHead )
	{
		m_pArgListHead = pNode;

		// ------------------------------------
		// Since this is the first argument on
		// the stack, it's offset is 0.
		// ------------------------------------
		pNode->m_nOffset = 0;

		// ------------------------------------
		// Increment the total number of args.
		// ------------------------------------
		m_nNumArgs++;

		// ------------------------------------
		// We are done here.
		// ------------------------------------
		return true;
	}

	// ------------------------------------
	// Loop through and find a node with
	// a free m_pNext. While doing this
	// ------------------------------------
	ArgNode_t* pTemp = m_pArgListHead;
	while( pTemp->m_pNext )
	{
		// ------------------------------------
		// We need to move onto the next node 
		// in the list.
		// ------------------------------------
		pTemp = pTemp->m_pNext;
	}

	// ------------------------------------
	// Setup pTemp.
	// ------------------------------------
	pTemp->m_pNext = pNode;

	// ------------------------------------
	// Increment total number of args.
	// ------------------------------------
	m_nNumArgs++;

	// ------------------------------------
	// Finished.
	// ------------------------------------
	return true;
}

// =======================================================================
// Retrieves an argument from the linked list.
// =======================================================================
ArgNode_t* CFuncStack::GetArgument( int pos )
{
	// ------------------------------------
	// Make sure we are not out of bounds.
	// ------------------------------------
	if( (pos < 0) || (pos > m_nNumArgs) )
	{
		return NULL;
	}

	// ------------------------------------
	// Make sure that the argument list
	// head is valid.
	// ------------------------------------
	if( !m_pArgListHead )
	{
		return NULL;
	}

	// ------------------------------------
	// Loop through the argument list.
	// Return the one at the desired 
	// position.
	// ------------------------------------
	ArgNode_t* pTemp = m_pArgListHead;
	for( int i = 0; i < m_nNumArgs; i++ )
	{
		// ------------------------------------
		// If this is true, we are at the
		// position of the desired argument.
		// Return it.
		// ------------------------------------
		if( i == pos )
		{
			return pTemp;
		}

		// ------------------------------------
		// If we are here, we need to move on
		// to the next argument.
		// ------------------------------------
		pTemp = pTemp->m_pNext;
	}

	// ------------------------------------
	// If we are here, we couldn't find
	// the argument in the list. This should
	// NEVER happen because we check to make
	// sure that pos is in range on line 37.
	// ------------------------------------
	return NULL;
}
