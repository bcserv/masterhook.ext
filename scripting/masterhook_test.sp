
// enforce semicolons after each code statement
#pragma semicolon 1

#include <sourcemod>
#include <masterhook>
#include <smlib>

#define PLUGIN_VERSION "0.1"



/*****************************************************************


		P L U G I N   I N F O


*****************************************************************/

public Plugin:myinfo = {
	name = "Masterhook Test Plugin",
	author = "Berni",
	description = "Masterhook Test Plugin",
	version = PLUGIN_VERSION,
	url = "http://www.mannisfunhouse.eu"
}



/*****************************************************************


		G L O B A L   V A R S


*****************************************************************/

// ConVar Handles

// Misc



/*****************************************************************


		F O R W A R D   P U B L I C S


*****************************************************************/

public OnPluginStart() {

	Mh_HookFunction("CBaseEntity::PhysicsMarkEntityAsTouched", ValveLibrary_Server, Hook_PhysicsMarkEntityAsTouched, MhDataType_Void);
	Mh_HookFunction("CAI_BaseNPC::HandleAnimEvent", ValveLibrary_Server, Hook_CAI_BaseNPCHandleAnimEvent, MhDataType_Void);
	//Mh_HookFunction("CPlayerLocalData::UpdateAreaBits", ValveLibrary_Server, Hook_CPlayerLocalDataUpdateArea, MhDataType_Void);
	Mh_HookFunction("Pickup_ForcePlayerToDropThisObject", ValveLibrary_Server, Hook_PickupForcePlayerToDropThi, MhDataType_Void);
	Mh_HookFunction("CBasePlayer::GetInVehicle", ValveLibrary_Server, Hook_GetInVehicle, MhDataType_Bool);
}



/****************************************************************


		C A L L B A C K   F U N C T I O N S


****************************************************************/

public Action:Hook_PhysicsMarkEntityAsTouched(&entity, &other) {
	
	if (Entity_ClassNameMatches(entity, "trigger_hurt") && Entity_ClassNameMatches(other, "prop_physics_respawnable")) {
		AcceptEntityInput(other, "Break");
		return Plugin_Handled;
	}
	
	return Plugin_Continue;
}

public Action:Hook_CAI_BaseNPCHandleAnimEvent(&entity, &unknown) {
	
	decl String:class[64];
	Entity_GetClassName(entity, class, sizeof(class));
	PrintToChatAll("\x04[MASTERHOOk] Hook_CAI_BaseNPCHandleAnimEvent: %s", class);
	
	if (Entity_ClassNameMatches(entity, "cycler", true)) {
		return Plugin_Handled;
	}
	
	return Plugin_Continue;
}

public Action:Hook_CPlayerLocalDataUpdateArea(&CPlayerLocalData, &client, &chAreaPortalBits) {
	
	if (!client) {
		return Plugin_Handled;
	}
	
	return Plugin_Continue;
}

public Action:Hook_PickupForcePlayerToDropThi(&client) {
	
	return Plugin_Handled;
}

public Action:Hook_GetInVehicle(&client, &vehicle, &role) {

	PrintToChatAll("\x04[DEBUG] Hook_GetInVehicle: %d %d", client, vehicle);
	return Plugin_Handled;
}

/*****************************************************************


		P L U G I N   F U N C T I O N S


*****************************************************************/

