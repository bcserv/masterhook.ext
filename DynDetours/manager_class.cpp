// =======================================================================
// File: manager_class.cpp
// Purpose: Implementation of CDetourManager.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "detour_class.h"
#include "manager_class.h"

// =======================================================================
// Adds a detour to the list.
// =======================================================================
CDetour* CDetourManager::CreateDetour(void* pTarget, void* pCallBack, 
									  eCallConv conv, char* szParams)
{
	// ------------------------------------
	// Make all given information is
	// valid.
	// ------------------------------------
	if( !pTarget || !pCallBack || !szParams ) {
		return NULL;
	}

	// ------------------------------------
	// We'll use this to either find or
	// create the detour.
	// ------------------------------------
	CDetour* pDetour = NULL;

	// ------------------------------------
	// Try to find the detour in the map.
	// ------------------------------------
	TDetourMap::iterator iterDetour = m_DetourList.find((unsigned int)pTarget);

	// ------------------------------------
	// Make sure it's valid...
	// ------------------------------------
	if( iterDetour == m_DetourList.end() ) {
		
		// ------------------------------------
		// If not, we need to create it.
		// ------------------------------------
		pDetour = new CDetour(pTarget, szParams, conv);

		// ------------------------------------
		// Now add it to the list
		// ------------------------------------
		m_DetourList.insert(TDetourPair((unsigned int)pTarget, pDetour));
	} else {

		// ------------------------------------
		// This means we found the detour.
		// Grab the instance.
		// ------------------------------------
		pDetour = iterDetour->second;
	}

	// ------------------------------------
	// Add the callback to it.
	// ------------------------------------
	pDetour->AddCallback( pCallBack );

	// ------------------------------------
	// Return the detour!
	// ------------------------------------
	return pDetour;
}
