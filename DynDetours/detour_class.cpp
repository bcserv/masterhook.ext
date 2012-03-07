// =======================================================================
// File: detour_class.cpp
// Purpose: This file contains the CDetour class which is responsible for
//	hooking a function and executing all of the callbacks.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "detour_class.h"
#include "asm.h"
#include "memutils.h"
#include "dd_utils.h"

// =======================================================================
// Constructor
// =======================================================================
CDetour::CDetour( void* pTarget, CFuncObj* funcObj )
{
	// ------------------------------------
	// Can't continue if any of these are 
	// not valid.
	// ------------------------------------
	if( !pTarget || !funcObj )
	{
		m_bInitialized = false;
		m_pFuncObj = NULL;
		m_pTrampoline = NULL;
		return;
	}

	m_pFuncObj = funcObj;

	// ------------------------------------
	// Make sure it is valid.
	// ------------------------------------
	if( !m_pFuncObj )
	{
		m_bInitialized = false;
		m_pTrampoline = NULL;
		return;
	}

	// ------------------------------------
	// We now need to create the trampoline
	// for the function.
	// ------------------------------------
	m_pTrampoline = new CTrampoline( pTarget );

	// ------------------------------------
	// Make sure it is valid.
	// ------------------------------------
	if( !m_pTrampoline )
	{
		m_bInitialized = false;
		return;
	}

	// ------------------------------------
	// Create ASM bridge here.
	// ------------------------------------
	m_pBridge = new CASMBridge( this );

	// ------------------------------------
	// Make sure we are initialized
	// ------------------------------------
	if( !m_pBridge->GetInitialized() )
	{
		return;
	}

	// ------------------------------------
	// Give ourselves write permissions
	// to the function.
	// ------------------------------------
	SetMemPatchable( pTarget, m_pTrampoline->GetNumSavedBytes() );

	// ------------------------------------
	// Now inject a jump to the ASM bridge.
	// ------------------------------------
	inject_jmp( pTarget, m_pBridge->GetBase() );
}

// =======================================================================
// Destructor.
// =======================================================================
CDetour::~CDetour( void )
{
	// ------------------------------------
	// Free up all of the callback managers.
	// ------------------------------------
	for( unsigned int i = 0; i < m_vecPreCallbacks.size(); i++ )
	{
		delete m_vecPreCallbacks[i];
	}

	for( unsigned int i = 0; i < m_vecPostCallbacks.size(); i++ )
	{
		delete m_vecPostCallbacks[i];
	}

	// ------------------------------------
	// Free up the trampoline.
	// ------------------------------------
	delete m_pTrampoline;

	// ------------------------------------
	// Free up the function object.
	// ------------------------------------
	delete m_pFuncObj;

	// ------------------------------------
	// Free up the ASM bridge.
	// ------------------------------------
	delete m_pBridge;
}

// =======================================================================
// Creates a function object from a string.
// =======================================================================
CFuncObj* CDetour::CreateFromString( const char* szParamList, eCallConv eConv, void* pTarget )
{
	// ------------------------------------
	// Can't continue if this is not valid.
	// ------------------------------------
	if( !szParamList || !pTarget )
	{
		return NULL;
	}

	// --------------------------------------------------------------
	// The format for a parameter list string is as follows:
	//	Types:
	//		i - Integer
	//		s - short
	//		S - string
	//		f - float
	//		d - double
	//		c - char
	//
	// Modifiers:
	//		[<types>] - Everything between the brackets passed by reference.
	//
	// Format:
	//		<types>)<ret-type> - All parameters go on the left side of the ')'.
	//
	// --------------------------------------------------------------

	// ------------------------------------
	// Some variables not local to the
	// loop that we will need.
	// ------------------------------------
	const char* pCurPos = szParamList;

	// ------------------------------------
	// Create the function object.
	// ------------------------------------
	CFuncObj* pFuncObj = new CFuncObj( pTarget, eConv );

	// ------------------------------------
	// If we're a thiscall, we need to
	// skip the first 'p' character.
	// ------------------------------------
	if( eConv == CONV_THISCALL ) {
		pCurPos++;
	}

	// ------------------------------------
	// Loop through each char.
	// ------------------------------------
	while( *pCurPos != ')' && *pCurPos != '\0' )
	{
		// ------------------------------------
		// Temporary loop variables.
		// ------------------------------------
		eArgPassType tmpPass = PASS_BYVAL;
		eArgType tmpType = CharToTypeEnum( *pCurPos );

		// ------------------------------------
		// Add the argument to our function
		// object.
		// ------------------------------------
		pFuncObj->AddArg( tmpType, tmpPass );

		// ------------------------------------
		// Move onto the next character.
		// ------------------------------------
		pCurPos++;
	}

	// ------------------------------------
	// If we're here, we've hit either
	// the end of the string ('\0') or
	// a ')'.
	// ------------------------------------
	if( *pCurPos == '\0' )
	{
		// ------------------------------------
		// No return type for us to use!
		// ------------------------------------
		printf("Parameter string doesn't have a return value!\n");
		return pFuncObj;
	}

	// ------------------------------------
	// If we've hit a ')', move to the next
	// char and figure out what the return
	// type is.
	// ------------------------------------
	if( *pCurPos == ')' )
	{
		// ------------------------------------
		// Move onto the return type char.
		// ------------------------------------
		pCurPos++;

		// ------------------------------------
		// Store the return type.
		// ------------------------------------
		eArgType tmpType;

		// ------------------------------------
		// Figure out the return value.
		// ------------------------------------
		switch( *pCurPos )
		{
			case 'v':
			{
				tmpType = TYPE_VOID;
				break;
			}

			case 'i':
			{
				tmpType = TYPE_INT32;
				break;
			}

			case 'p':
			{
				tmpType = TYPE_INT32_PTR;
				break;
			}

			case 'f':
			{
				tmpType = TYPE_FLOAT;
				break;
			}
		}

		// ------------------------------------
		// Set the return type.
		// ------------------------------------
		pFuncObj->SetRetType( tmpType );
	}

	// ------------------------------------
	// Done!
	// ------------------------------------
	return pFuncObj;
}

// =======================================================================
// Adds a callback manager.
// =======================================================================
void CDetour::AddManager( ICallbackManager* pManager, eHookType type )
{
	// ------------------------------------
	// Sanity check.
	// ------------------------------------
	if( !pManager ) {
		return;
	}

	// ------------------------------------
	// Add the callback manager to the
	// proper internal list.
	// ------------------------------------
	switch( type ) 
	{

		case TYPE_PRE:
			m_vecPreCallbacks.push_back( pManager );
			break;
		
		case TYPE_POST:
			m_vecPostCallbacks.push_back( pManager );
			break;
	}

}

// =======================================================================
// Returns the requested callback manager.
// =======================================================================
ICallbackManager* CDetour::GetManager( const char* lang, eHookType type )
{
	// ------------------------------------
	// Sanity check
	// ------------------------------------
	if( !lang ) {
		return NULL;
	}

	// ------------------------------------
	// Find the list to search.
	// ------------------------------------
	std::vector<ICallbackManager *>* pList = NULL;
	switch( type ) 
	{
		case TYPE_PRE:
			pList = &m_vecPreCallbacks;
			break;

		case TYPE_POST:
			pList = &m_vecPostCallbacks;
			break;
	}

	// ------------------------------------
	// Loop through the list.
	// ------------------------------------
	if( pList ) 
	{
		for( unsigned int i = 0; i < pList->size(); i++ ) {
			// ------------------------------------
			// Get the callback at this address.
			// ------------------------------------
			ICallbackManager* pMan = (*pList)[i];

			// ------------------------------------
			// Return it if the names match.
			// ------------------------------------
			if( strcmp(lang, pMan->GetLang()) == 0 ) {
				return pMan;
			}
		}
	}

	// ------------------------------------
	// Something went wrong.
	// ------------------------------------
	return NULL;
}

// =======================================================================
// Executes all callbacks.
// =======================================================================
HookRetBuf_t* CDetour::DoCallbacks( eHookType type )
{
	// ------------------------------------
	// Pointer to the callback list we will
	// use.
	// ------------------------------------
	vector<ICallbackManager *>* pList = NULL;

	// ------------------------------------
	// Figure out which list to loop through.
	// ------------------------------------
	switch( type ) 
	{
		// ------------------------------------
		// Pre callbacks.
		// ------------------------------------
		case TYPE_PRE:
			pList = &m_vecPreCallbacks;
			break;

		// ------------------------------------
		// Post callbacks.
		// ------------------------------------
		case TYPE_POST:
			pList = &m_vecPostCallbacks;
			break;
	}

	// ------------------------------------
	// Get the size of the return value.
	// ------------------------------------
	int iRetValSize = 
		GetFuncObj()->GetRetType()->GetSize();

	// ------------------------------------
	// Will hold the final action and
	// return value after all hooks are
	// executed.
	// ------------------------------------
	HookRetBuf_t* pFinalBuf = new HookRetBuf_t;
	
	// ------------------------------------
	// Set the initial values.
	// ------------------------------------
	pFinalBuf->eRes	   = HOOKRES_NONE;
	pFinalBuf->pRetBuf = NULL;

	// ------------------------------------
	// Start executing all of the functions.
	// ------------------------------------
	for( unsigned int i = 0; i < pList->size(); i++ )
	{
		// ------------------------------------
		// Temporary hook buffer.
		// ------------------------------------
		HookRetBuf_t* pTempBuf = NULL;

		// ------------------------------------
		// Get the callback manager.
		// ------------------------------------
		ICallbackManager* pMan = (ICallbackManager *)((*pList)[i]);
		
		// ------------------------------------
		// Figure out function to call.
		// ------------------------------------
		switch( type ) 
		{
			// ------------------------------------
			// Pre callbacks.
			// ------------------------------------
			case TYPE_PRE:
				pTempBuf = pMan->DoPreCalls( this );
				break;

			// ------------------------------------
			// Post callbacks.
			// ------------------------------------
			case TYPE_POST:
				pTempBuf = pMan->DoPostCalls( this );
				break;
		}

		// ------------------------------------
		// Sanity checking.
		// ------------------------------------
		if( !pTempBuf ) {
			continue;
		}

		// ------------------------------------
		// Prioritize the actions.
		// ------------------------------------
		if( pFinalBuf->eRes <= pTempBuf->eRes ) {

			// ------------------------------------		
			// Copy the action
			// ------------------------------------
			pFinalBuf->eRes = pTempBuf->eRes;
			pFinalBuf->pRetBuf = pTempBuf->pRetBuf;
		}

		// ------------------------------------
		// Free up memory.
		// ------------------------------------
		free( pTempBuf );
	}

	// ------------------------------------
	// Return the highest priority action.
	// ------------------------------------
	return pFinalBuf;
}
