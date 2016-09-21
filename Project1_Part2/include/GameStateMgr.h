// ---------------------------------------------------------------------------
// Project Name		:	Asteroid
// File Name		:	GameStateMgr.h
// Author			:	Sun Tjen Fam
// Creation Date	:	2007/10/26
// Purpose			:	header file for the game state manager
// History			:
// - 2008/02/08		:   - updated to be used in conjuction with the build in
//						  game state manager in the Alpha Engine.
// - 2007/10/26		:	- initial implementation
// ---------------------------------------------------------------------------

#ifndef GAME_STATE_MGR_H
#define GAME_STATE_MGR_H

// ---------------------------------------------------------------------------

#include "AEEngine.h"

// ---------------------------------------------------------------------------
// include the list of game states

#include "GameStateList.h"

// ---------------------------------------------------------------------------
// externs

extern unsigned int gGameStateNext;


// ---------------------------------------------------------------------------
// Function prototypes

// call this at the beginning and AFTER all game states are added to the manager
void GameStateMgrInit(unsigned int gameStateInit);

// update is used to set the function pointers
void GameStateMgrUpdate();

// Main flow
void GSM_MainLoop(void);

// ---------------------------------------------------------------------------

#endif // AE_GAME_STATE_MGR_H