// =======================================================================
// File: arg_class.cpp
// Purpose: Provides implementations of methods declared in arg_class.h
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "arg_class.h"

// =======================================================================
// Constructor
// =======================================================================
CFuncArg::CFuncArg( void )
{
	// ------------------------------------
	// Setup initial values.
	// ------------------------------------
	m_ArgType = TYPE_UNKNOWN;
	m_PassType = PASS_UNKNOWN;
	m_nSize = 0;
}

// =======================================================================
// Sets the type of this argument.
// =======================================================================
void CFuncArg::SetType( eArgType type )
{
	m_ArgType = type; 

	// ------------------------------------
	// If we're pass by reference, assume
	// size is sizeof(void *).
	// ------------------------------------
	if( m_PassType == PASS_BYREF )
	{
		m_nSize = sizeof(void *);
		return;
	}

	// ------------------------------------
	// Figure out how big the variable is.
	// ------------------------------------
	switch( type )
	{
		case TYPE_CHAR:
		case TYPE_INT8:
			m_nSize = sizeof(char);
			break;

		case TYPE_INT16:
			m_nSize = sizeof(short);
			break;

		case TYPE_BOOL:
		case TYPE_INT32:
			m_nSize = sizeof(int);
			break;

		case TYPE_FLOAT:
			m_nSize = sizeof(float);
			break;

		case TYPE_INT64:
			m_nSize = sizeof(__int64);
			break;

		case TYPE_CHAR_PTR:
		case TYPE_FLOAT_PTR:
		case TYPE_INT8_PTR:
		case TYPE_INT16_PTR:
		case TYPE_INT32_PTR:
			m_nSize = sizeof(unsigned char *);
			break;

		default:
			m_nSize = 0;
			break;
	}
}
