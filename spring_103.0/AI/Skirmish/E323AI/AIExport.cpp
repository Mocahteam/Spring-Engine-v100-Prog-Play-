/*
	Copyright 2008  Nicolas Wu

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	@author Nicolas Wu
	@author Robin Vobruba <hoijui.quaero@gmail.com>
*/

#if WIN32
	#include <windows.h>
	#include <tchar.h>
#endif

#include <map>

// AI interface stuff
#include "ExternalAI/Interface/SSkirmishAILibrary.h"
#include "ExternalAI/Interface/SSkirmishAICallback.h"
#include "LegacyCpp/AIAI.h"
//#include "Game/GameVersion.h"

// E323AI stuff
#include "AIExport.h"
#include "CE323AI.h"

// NOTE: myAIs is not static cause we need to count AI instances from outside

// skirmishAIId -> AI map
static std::map<int, CAIAI*> myAIs;

// filled in init() of the first instance of this AI
static const char* aiVersion = NULL;

// callbacks for all the instances controlled by this Skirmish AI
static std::map<int, const struct SSkirmishAICallback*> skirmishAIId_callback;


/*
EXPORT(enum LevelOfSupport) getLevelOfSupportFor(
		const char* aiShortName, const char* aiVersion,
		const char* engineVersionString, int engineVersionNumber,
		const char* aiInterfaceShortName, const char* aiInterfaceVersion) {

	const char* springVersion = SpringVersion::GetFull().c_str();
	const int cmp = strcmp(engineVersionString, springVersion);

	if (cmp == 0 && engineVersionNumber <= ENGINE_VERSION_NUMBER) {
		return LOS_Working;
	}

	return LOS_None;
}
*/

EXPORT(int) init(int skirmishAIId, const struct SSkirmishAICallback* callback) {

	if (myAIs.count(skirmishAIId) > 0) {
		// the map has no AI for this skirmishAIId.
		// raise an error, since it's probably a mistake if we are trying to
		// release a skirmishAIId that's not initialized.
		return -1;
	}

	if (aiVersion == NULL) {
		aiVersion = callback->SkirmishAI_Info_getValueByKey(skirmishAIId, SKIRMISH_AI_PROPERTY_VERSION);
	}

	skirmishAIId_callback[skirmishAIId] = callback;

	// CAIAI is the Legacy C++ wrapper
	myAIs[skirmishAIId] = new CAIAI(new CE323AI());

	// signal: everything went ok
	return 0;
}

EXPORT(int) release(int skirmishAIId) {

	if (myAIs.find(skirmishAIId) == myAIs.end()) {
		// the map has no AI for this skirmishAIId.
		// raise an error, since it's probably a mistake if we're trying to
		// release a team that's not initialized.
		return -1;
	}

	delete myAIs[skirmishAIId];
	myAIs[skirmishAIId] = NULL;
	myAIs.erase(skirmishAIId);

	skirmishAIId_callback.erase(skirmishAIId);

	// signal: everything went ok
	return 0;
}

EXPORT(int) handleEvent(int skirmishAIId, int topic, const void* data) {

	if (skirmishAIId < 0) {
		// events sent to skirmishAIId -1 will always be to the AI object itself,
		// not to a particular skirmishAIId.
	} else if (myAIs.find(skirmishAIId) != myAIs.end()) {
		// allow the AI instance to handle the event.
		return myAIs[skirmishAIId]->handleEvent(topic, data);
	}

	// no AI for that skirmishAIId, so return error.
	return -1;
}

// methods from here on are for AI internal use only

const char* aiexport_getVersion() {
	return aiVersion;
}

const char* aiexport_getMyOption(int skirmishAIId, const char* key) {
	return skirmishAIId_callback[skirmishAIId]->SkirmishAI_OptionValues_getValueByKey(skirmishAIId, key);
}

size_t aiexport_getNumAIInstances() {
	return myAIs.size();
}

size_t aiexport_getNumAIInstancesInAllyTeam(int skirmishAIId, int allyTeamId) {

	size_t numAllies = 0;

#if !defined(BUILDING_AI_FOR_SPRING_0_81_2)
	std::map<int, CAIAI*>::iterator aii;
	for (aii = myAIs.begin(); aii != myAIs.end(); ++aii) {
		const int otherSkirmishAIId = aii->first;
		const int otherTeamId = skirmishAIId_callback[otherSkirmishAIId]->Game_getMyTeam(otherSkirmishAIId);
		if (skirmishAIId_callback[skirmishAIId]->Game_getTeamAllyTeam(skirmishAIId, otherTeamId) == allyTeamId) {
			numAllies++;
		}
	}
#else
	numAllies++;
#endif

	return numAllies;
}

bool aiexport_isPrimaryInstance(int skirmishAIId) {
	return (myAIs.begin()->first == skirmishAIId);
}
