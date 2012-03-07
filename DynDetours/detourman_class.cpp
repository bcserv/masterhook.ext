/**
* ======================================================
* DynDetours
* Copyright (C) 2009 Deniz Sezen
* All rights reserved.
* ======================================================
*
* This software is provided 'as-is', without any express or implied warranty.
* In no event will the authors be held liable for any damages arising from 
* the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose, 
* including commercial applications, and to alter it and redistribute it 
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not 
* claim that you wrote the original software. If you use this software in a 
* product, an acknowledgment in the product documentation would be 
* appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source distribution.
*
*/

//========================================================================
// Includes
//========================================================================
#include "detourman_class.h"
#include "detour_class.h"
#include "extension.h"

//========================================================================
// Global variables
//========================================================================
CDetourManager g_DetourManager;

//========================================================================
// Destructor
//========================================================================
CDetourManager::~CDetourManager()
{
	// Loop through each element in the vector and get rid of it.
	for( unsigned int i = 0; i < m_DetourList.size(); i++ )
	{
		// Get the detour instance.
		CDetour* pCurDetour = m_DetourList[i];

		// Erase it
		delete pCurDetour;
	}
}

//========================================================================
// Adds a detour.
//========================================================================
CDetour* CDetourManager::Add_Detour( void* pTarget, CFuncObj* funcObj, eCallConv conv )
{
	// We need both a target and a parameter string
	// to continue.
	if( !pTarget || !funcObj)
		return NULL;

	META_CONPRINTF("[MASTERHOOK] Error 1 in CDetourManager::Add_Detour()\n");

	// Do we already have a function with this address?
	CDetour* pDetour = Find_Detour(pTarget);

	// Return it if there already is one.
	if( pDetour )
	{
		return pDetour;
	}

	// Otherwise, we've got to make one
	pDetour = new CDetour( pTarget, funcObj );
	META_CONPRINTF("[MASTERHOOK] %X %X\n", pTarget, funcObj);

	// Add them to the list
	m_DetourList.push_back(pDetour);

	// Return the newly created detour
	return pDetour;
}

//========================================================================
// Removes a detour.
//========================================================================
bool CDetourManager::Remove_Detour( void* pTarget )
{
	// Need at least a target address to find the detour instance.
	if( !pTarget )
	{
		return false;
	}

	// Loop through the list
	for( unsigned int i = 0; i < m_DetourList.size(); i++ )
	{
		// Did we find it?
		if( m_DetourList[i] == pTarget ) {
			
			// Free the detour.
			delete m_DetourList[i];

			// Erase it from the vector.
			m_DetourList.erase( m_DetourList.begin() + i );

			// Done.
			return true;
		}
	}

	// If we get here, we didn't find it.
	return false;
}

//========================================================================
// Finds a detour in the list by its target function's address.
//========================================================================
CDetour* CDetourManager::Find_Detour( void *pTarget )
{
	// Sanity check.
	if( !pTarget ) {
		return NULL;
	}

	// Loop through the list, see if we can find it
	for( unsigned int i = 0; i < m_DetourList.size(); i++ )
	{
		// Get the detour
		CDetour* pDetour = m_DetourList[i];

		// Is it valid?
		if( !pDetour )
			continue;

		// Is it the right one?
		if( pDetour->GetFuncObj()->GetAddress() == pTarget )
			return pDetour;
	}

	// If we make it here, we can't find it.
	return NULL;
}
