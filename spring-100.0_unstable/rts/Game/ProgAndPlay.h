/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef __PROG_AND_PLAY_H__
#define __PROG_AND_PLAY_H__

// Muratet (Define Class CProgAndPlay) ---

#include "Sim/Units/Unit.h"

class CProgAndPlay
{
public:
	CProgAndPlay();
	~CProgAndPlay();
	
	void Update(void);
	
	void AddUnit(CUnit* unit);
	void UpdateUnit(CUnit* unit);
	void RemoveUnit(CUnit* unit);

private:
	bool loaded;
	bool updated;
	
	int updatePP(); // update Prog&Play data if necessary
	int execPendingCommands(); // execute pending command from Prog&Play
	
};

extern CProgAndPlay* pp;

// ---

#endif // __PROG_AND_PLAY_H__
