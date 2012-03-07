// =======================================================================
// File: func_class.cpp
// Purpose: Defines the methods declared in func_class.h.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "func_class.h"
#include "dd_utils.h"

// =======================================================================
// Constructor.
// =======================================================================
CFuncObj::CFuncObj( void* pAddr, eCallConv conv )
{
	// ------------------------------------
	// Setup information about the function
	// itself.
	// ------------------------------------
	m_pAddr = pAddr;
	m_eConv = conv;
}

// =======================================================================
// Constructor 2.
// =======================================================================
CFuncObj::CFuncObj( void* pAddr, const char* szParamList, eCallConv eConv )
{
	// ------------------------------------
	// Store the address and calling conv.
	// ------------------------------------
	m_pAddr = pAddr;
	m_eConv = eConv;

	// ------------------------------------
	// Now we need to parse the parameter
	// format string and add the arguments.
	// ------------------------------------

	// ------------------------------------
	// Can't continue if the list is not
	// valid.
	// ------------------------------------
	if( !szParamList )
	{
		return;
	}

	// ------------------------------------
	// Create a pointer to the beginning
	// of the list.
	// ------------------------------------
	const char* pCurChar = szParamList;

	// ------------------------------------
	// If we hit a '[' character, everything
	// that comes after it is pass_byref.
	// Default to PASS_BYVAL until we hit
	// one.
	// ------------------------------------
	eArgPassType passType = PASS_BYVAL;

	// ------------------------------------
	// Start looping through.
	// ------------------------------------
	while( *pCurChar != '\0' && *pCurChar != ')' )
	{
		// ------------------------------------
		// Check for any special characters.
		// ------------------------------------
		switch( *pCurChar )
		{
			// ------------------------------------
			// Pass byref!
			// ------------------------------------
			case '[':
				passType = PASS_BYREF;
				break;

			// ------------------------------------
			// Pass byval!
			// ------------------------------------
			case ']':
				passType = PASS_BYVAL;
				break;
		}

		// ------------------------------------
		// Create a function arg instance.
		// ------------------------------------
		CFuncArg* pArg = new CFuncArg();

		// ------------------------------------
		// Now setup the data.
		// ------------------------------------
		pArg->SetType( CharToTypeEnum(*pCurChar) );
		pArg->SetPassType(passType);

		// ------------------------------------
		// Add the argument to the list.
		// ------------------------------------
		m_Stack.AddArgument( pArg );
		// m_ArgList.push_back( pArg );

		// ------------------------------------
		// Move onto the next character.
		// ------------------------------------
		pCurChar++;
	}
}

// =======================================================================
// Destructor
// =======================================================================
CFuncObj::~CFuncObj( void )
{
	// ------------------------------------
	// Do nothing.
	// ------------------------------------
}

// =======================================================================
// Returns the number of arguments registered to this function object.
// =======================================================================
unsigned int CFuncObj::GetNumArgs( void )
{
	return m_Stack.GetNumArgs();
}

// =======================================================================
// Returns the argument at the specified index.
// =======================================================================
CFuncArg* CFuncObj::GetArg( int iArgNum )
{
	// ------------------------------------
	// TODO: NEED ERROR CHECKING HERE.
	// ------------------------------------
	return m_Stack.GetArgument( iArgNum )->m_pArg;
}

// =======================================================================
// Returns a pointer to our stack.
// =======================================================================
CFuncStack* CFuncObj::GetStack( void )
{
	return &m_Stack;
}

// =======================================================================
// Adds an argument.
// =======================================================================
void CFuncObj::AddArg( eArgType type, eArgPassType passType, int size/* =0 */)
{
	// ------------------------------------
	// Don't add void as an argument.
	// ------------------------------------
	if( type == TYPE_VOID )
	{
		return;
	}

	// ------------------------------------
	// Construct a function argument.
	// ------------------------------------
	CFuncArg* pArg = new CFuncArg();

	// ------------------------------------
	// Register the appropriate info.
	// ------------------------------------
	pArg->SetType( type );

	// ------------------------------------
	// Set the pass type.
	// ------------------------------------
	pArg->SetPassType( passType );

	// ------------------------------------
	// If we're passing byref, assume a
	// size of 4.
	// ------------------------------------
	if( passType == PASS_BYREF ) {
		pArg->SetSize( sizeof(void *) );
	}

	// ------------------------------------
	// Otherwise, if we're an unknown type,
	// we'll need the size manually.
	// ------------------------------------
	else if( type == TYPE_UNKNOWN ) {
		pArg->SetSize( size );
	}

	// ------------------------------------
	// Add the argument to our internal
	// list.
	// ------------------------------------
	m_Stack.AddArgument( pArg );
	// m_ArgList.push_back(pArg);
}

// =======================================================================
// Manually sets up the return type for this function.
// =======================================================================
void CFuncObj::SetRetType( eArgType retType, int size/*=0 */ )
{
	// ------------------------------------
	// Set the return value type.
	// ------------------------------------
	m_RetArg.SetType( retType );

	// ------------------------------------
	// Only have to do this if it's a custom
	// type.
	// ------------------------------------
	if( retType == TYPE_UNKNOWN )
	{
		m_RetArg.SetSize( size );
	}
}
