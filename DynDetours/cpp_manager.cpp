// =======================================================================
// File: cpp_manager.cpp
// Purpose: Defines the CCPPManager callback manager. This class is
//	responsible for adding / creating callbacks for the C++ language.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "cpp_manager.h"
#include "detour_class.h"
#include "detourman_class.h"

// =======================================================================
// This will create a C++ function callback for a detour.
// =======================================================================
bool CPP_CreateCallback( void* target, eCallConv conv, const char* szParams,
						CPPCallBack callback, eHookType type )
{
	// ------------------------------------
	// Can't continue if any of these are
	// null.
	// ------------------------------------
	if( !callback ) {
		return false;
	}

	// ------------------------------------
	// Check to see if a detour already
	// exists at this function address.
	// ------------------------------------
	CDetour* pDetour = g_DetourManager.Find_Detour( target );

	// ------------------------------------
	// If the detour is not valid,
	// we need to create it and add it
	// to the manager.
	// ------------------------------------
	if( !pDetour ) {
		//pDetour = g_DetourManager.Add_Detour( target, szParams, conv );
	}

	// ------------------------------------
	// Check to see if the detour already
	// has a callback manager.
	// ------------------------------------
	ICallbackManager* pMan = pDetour->GetManager("C++", type);
	
	// ------------------------------------
	// If not, we need to create one and
	// add it to the detour.
	// ------------------------------------
	if( !pMan )
	{
		pMan = new CCPPManager();
		pDetour->AddManager( pMan, type );
	}

	// ------------------------------------
	// Now add the callback.
	// ------------------------------------
	pMan->Add( (void*)callback, type );

	// ------------------------------------
	// Done.
	// ------------------------------------
	return true;
}

// =======================================================================
// Adds a callback to the list.
// =======================================================================
void CCPPManager::Add( void* pFuncObj, eHookType type )
{
	// ------------------------------------
	// Sanity checking.
	// ------------------------------------
	if( !pFuncObj ) {
		return;
	}

	// ------------------------------------
	// Add the callback to the proper list.
	// ------------------------------------
	switch( type ) 
	{
		case TYPE_PRE:
			m_vecPreCalls.push_back( pFuncObj );
			break;
			
		case TYPE_POST:
			m_vecPostCalls.push_back( pFuncObj );
			break;
	}
}

// =======================================================================
// Removes a callback from the list.
// =======================================================================
void CCPPManager::Remove( void* pFuncObj, eHookType type )
{
	// ------------------------------------
	// Sanity checking.
	// ------------------------------------
	if( !pFuncObj ) {
		return;
	}

	// ------------------------------------
	// Find the proper callback list to
	// look in.
	// ------------------------------------
	std::vector<void *>* pList = NULL;
	switch( type )
	{
		case TYPE_PRE:
			pList = &m_vecPreCalls;
			break;

		case TYPE_POST:
			pList = &m_vecPostCalls;
			break;
	}

	// ------------------------------------
	// Using iterators, loop through the
	// list and delete the function.
	// ------------------------------------
	for( unsigned int i = 0; i < pList->size(); i++ )
	{
		// ------------------------------------
		// Compare the function item at this
		// index.
		// ------------------------------------
		if( (*pList)[i] == pFuncObj ) 
		{
			// ------------------------------------
			// If they match, remove it.
			// ------------------------------------
			pList->erase( pList->begin() + i );

			// ------------------------------------
			// Done.
			// ------------------------------------
			break;
		}
	}
}

// =======================================================================
// Processes pre-callbacks.
// =======================================================================
HookRetBuf_t* CCPPManager::DoPreCalls( CDetour* pDet )
{
	// ------------------------------------
	// Sanity checks.
	// ------------------------------------
	if( !pDet ) {
		return NULL;
	}

	// ------------------------------------
	// The temporary return value buffer.
	// ------------------------------------
	HookRetBuf_t* pFinalBuf = new HookRetBuf_t;
	
	// ------------------------------------
	// Set some initial values.
	// ------------------------------------
	pFinalBuf->eRes	  = HOOKRES_NONE;
	pFinalBuf->pRetBuf = NULL;

	// ------------------------------------
	// Get the size of the return value.
	// ------------------------------------
	int iRetSize = 
		pDet->GetFuncObj()->GetRetType()->GetSize();

	// ------------------------------------
	// Start executing all of the functions.
	// ------------------------------------
	for( unsigned int i = 0; i < m_vecPreCalls.size(); i++ )
	{
		// ------------------------------------
		// Stores the return information from
		// the callback.
		// ------------------------------------
		HookRetBuf_t* pTempBuf = NULL;

		// ------------------------------------
		// Get the callback.
		// ------------------------------------
		CPPCallBack theCallback = (CPPCallBack)m_vecPreCalls[i];

		// ------------------------------------
		// Call the function.
		// ------------------------------------
		pTempBuf = theCallback( pDet );

		// ------------------------------------
		// Prioritize the actions.
		// ------------------------------------
		if( pFinalBuf->eRes <= pTempBuf->eRes ) {

			// ------------------------------------		
			// Copy the action and return value.
			// ------------------------------------
			pFinalBuf->eRes = pTempBuf->eRes;
			pFinalBuf->pRetBuf = pTempBuf->pRetBuf;
		}

		// ------------------------------------
		// Free the temp buffer.
		// ------------------------------------
		delete pTempBuf;
	}

	// ------------------------------------
	// Return the highest priority action.
	// ------------------------------------
	return pFinalBuf;
}

// =======================================================================
// Processes pre-callbacks.
// =======================================================================
HookRetBuf_t* CCPPManager::DoPostCalls( CDetour* pDet )
{
	// ------------------------------------
	// Sanity checks.
	// ------------------------------------
	if( !pDet ) {
		return NULL;
	}

	// ------------------------------------
	// The temporary return value buffer.
	// ------------------------------------
	HookRetBuf_t* pFinalBuf = new HookRetBuf_t;

	// ------------------------------------
	// Set some initial values.
	// ------------------------------------
	pFinalBuf->eRes	  = HOOKRES_NONE;
	pFinalBuf->pRetBuf = NULL;

	// ------------------------------------
	// Get the size of the return value.
	// ------------------------------------
	int iRetSize = 
		pDet->GetFuncObj()->GetRetType()->GetSize();

	// ------------------------------------
	// Start executing all of the functions.
	// ------------------------------------
	for( unsigned int i = 0; i < m_vecPostCalls.size(); i++ )
	{
		// ------------------------------------
		// Stores the return information from
		// the callback.
		// ------------------------------------
		HookRetBuf_t* pTempBuf = NULL;

		// ------------------------------------
		// Get the callback.
		// ------------------------------------
		CPPCallBack theCallback = (CPPCallBack)m_vecPostCalls[i];

		// ------------------------------------

		// Call the function.
		// ------------------------------------
		pTempBuf = theCallback( pDet );

		// ------------------------------------
		// Prioritize the actions.
		// ------------------------------------
		if( pFinalBuf->eRes <= pTempBuf->eRes ) {

			// ------------------------------------		
			// Copy the action
			// ------------------------------------
			pFinalBuf->eRes = pTempBuf->eRes;

			// ------------------------------------
			// Copy the return value contents if
			// the function has a return value.
			// ------------------------------------
			if( iRetSize ) {
				memcpy( pFinalBuf->pRetBuf, pTempBuf->pRetBuf, iRetSize );
			}
		}

		// ------------------------------------
		// Free the temp buffer.
		// ------------------------------------
		free( pTempBuf );
	}

	// ------------------------------------
	// Return the highest priority action.
	// ------------------------------------
	return pFinalBuf;
}
