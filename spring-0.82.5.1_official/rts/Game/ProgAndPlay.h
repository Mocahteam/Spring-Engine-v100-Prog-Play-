/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef __PROG_AND_PLAY_H__
#define __PROG_AND_PLAY_H__

// used to define a delay (in seconds) to accept units idled
#define UNITS_IDLED_TRIGGER 3
// used to define a delay (in seconds) to accept mission end
#define MISSION_ENDED_TRIGGER 2

// Muratet (Define Class CProgAndPlay) ---

#include "Sim/Units/Unit.h"
#include "Sim/Misc/GlobalConstants.h"
#include <ctime>
#include "lib/pp/traces/TracesParser.h"
#include "lib/pp/traces/TracesAnalyser.h"
#include "Lua/LuaHandle.h"
#include "System/FileSystem/VFSHandler.h"
#include "System/FileSystem/ArchiveScanner.h"
#include "System/ConfigHandler.h"
#include <boost/thread.hpp>
#ifdef _WIN32
#include <windows.h>
#endif
#include <boost/asio.hpp>

class CProgAndPlay
{
public:

	CProgAndPlay();
	~CProgAndPlay();

	void Update(void);
	void GamePaused(bool paused);
	void TracePlayer();
	void UpdateTimestamp();

	void AddUnit(CUnit* unit);
	void UpdateUnit(CUnit* unit);
	void RemoveUnit(CUnit* unit);

private:

	bool loaded;
	bool updated;
	bool newExecutionDetected;
	bool endExecutionDetected;
	bool missionEnded;
	bool traceModuleCorrectlyInitialized; // defines if trace module is correctly initialized
	bool tracesComing;
	bool testMapMode; // defines if we are in testing mode
	bool onGoingCompression;
	std::string photoFilename;
	std::string archivePath;
	std::string missionName;
	std::string lang;
	std::time_t startTime;
	boost::thread tracesThread;
	std::shared_ptr<TracesParser> tp;
	TracesAnalyser ta;

	int updatePP(); // update Prog&Play data if necessary
	int execPendingCommands(); // execute pending command from Prog&Play
	void logMessages(bool unitsIdled); // log messages from Prog&Play
	void initTracesFile(); // init and open the appropriate traces file based on the current mission
	bool allUnitsIdled(); // returns true if all units' command queues are empty (units of the player)
	bool allUnitsDead(); // returns true if the player has no units left in the game

	void sendFeedback(std::string feedback);
	std::string loadFile(std::string full_path);
	std::string loadFileFromVfs(std::string full_path);
	void publishOnFacebook();
	void sendRequestToServer();
	void openFacebookUrl();
};

static int units_idled_frame_counter = -1;
static int mission_ended_frame_counter = -1;

extern CProgAndPlay* pp;

// ---

#endif // __PROG_AND_PLAY_H__
