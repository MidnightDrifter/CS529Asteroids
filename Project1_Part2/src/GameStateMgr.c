// ---------------------------------------------------------------------------
// Project Name		:	Asteroid
// File Name		:	GameStateMgr.cpp
// Author			:	Sun Tjen Fam
// Creation Date	:	2008/01/31
// Purpose			:	implementation of the game state manager
// History			:
// - 2008/02/08		:	- modified to be used like the old style game state
//						  manager
// - 2008/01/31		:	- initial implementation
// - 2015/12/10		:	- Moved game flow from "main.c" to the "GSM_MainLoop" function 
// ---------------------------------------------------------------------------

#include "GameStateMgr.h"
#include "GameState_Asteroids.h"

// ---------------------------------------------------------------------------
// globals

// variables to keep track the current, previous and next game state
static unsigned int	gGameStateInit;
static unsigned int	gGameStateCurr;
static unsigned int	gGameStatePrev;
unsigned int	gGameStateNext;

// pointer to functions for game state life cycles functions
void (*GameStateLoad)(void)		= 0;
void (*GameStateInit)(void)		= 0;
void (*GameStateUpdate)(void)	= 0;
void (*GameStateDraw)(void)		= 0;
void (*GameStateFree)(void)		= 0;
void (*GameStateUnload)(void)	= 0;

// ---------------------------------------------------------------------------
// Functions implementations

void GameStateMgrInit(unsigned int gameStateInit)
{
	// set the initial game state
	gGameStateInit = gameStateInit;

	// reset the current, previoud and next game
	gGameStateCurr = 
	gGameStatePrev = 
	gGameStateNext = gGameStateInit;

	// call the update to set the function pointers
	GameStateMgrUpdate();
}

// ---------------------------------------------------------------------------

void GameStateMgrUpdate()
{
	if ((gGameStateCurr == GS_RESTART) || (gGameStateCurr == GS_QUIT))
		return;

	switch (gGameStateCurr)
	{
	case GS_ASTEROIDS:
		GameStateLoad = GameStateAsteroidsLoad;
		GameStateInit = GameStateAsteroidsInit;
		GameStateUpdate = GameStateAsteroidsUpdate;
		GameStateDraw = GameStateAsteroidsDraw;
		GameStateFree = GameStateAsteroidsFree;
		GameStateUnload = GameStateAsteroidsUnload;
		break;

	default:
		AE_FATAL_ERROR("invalid state!!");
	}
}

// ---------------------------------------------------------------------------


void GSM_MainLoop(void)
{
	while (gGameStateCurr != GS_QUIT)
	{
		// reset the system modules
		AESysReset();

		// If not restarting, load the gamestate
		if (gGameStateCurr != GS_RESTART)
		{
			GameStateMgrUpdate();
			GameStateLoad();
		}
		else
			gGameStateNext = gGameStateCurr = gGameStatePrev;

		// Initialize the gamestate
		GameStateInit();

		while (gGameStateCurr == gGameStateNext)
		{
			AESysFrameStart();

			AEInputUpdate();

			GameStateUpdate();

			GameStateDraw();

			AESysFrameEnd();

			// check if forcing the application to quit
			if ((0 == AESysDoesWindowExist()) || AEInputCheckTriggered(VK_ESCAPE))
				gGameStateNext = GS_QUIT;
		}

		GameStateFree();

		if (gGameStateNext != GS_RESTART)
			GameStateUnload();

		gGameStatePrev = gGameStateCurr;
		gGameStateCurr = gGameStateNext;
	}
}


// ---------------------------------------------------------------------------
