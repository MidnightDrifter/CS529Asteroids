// ---------------------------------------------------------------------------
// Project Name		:	Omega Library Test Program
// File Name		:	main.h
// Author			:	Sun Tjen Fam
// Creation Date	:	2007/04/26
// Purpose			:	main entry point for the test program
// History			:
// - 2007/04/26		:	- initial implementation
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// includes

#include "main.h"


// ---------------------------------------------------------------------------
// Static function protoypes


// ---------------------------------------------------------------------------
// main

int WINAPI WinMain(HINSTANCE instanceH, HINSTANCE prevInstanceH, LPSTR command_line, int show)
{
	// Initialize the system 
	AESysInitInfo sysInitInfo;

	sysInitInfo.mAppInstance		= instanceH;
	sysInitInfo.mShow				= show;
	sysInitInfo.mWinWidth			= 800; 
	sysInitInfo.mWinHeight			= 600;
	sysInitInfo.mCreateConsole		= 1;
	sysInitInfo.mMaxFrameRate		= 60;
	sysInitInfo.mpWinCallBack		= NULL;//MyWinCallBack;
	sysInitInfo.mClassStyle			= CS_HREDRAW | CS_VREDRAW;											
	sysInitInfo.mWindowStyle		= WS_OVERLAPPEDWINDOW;//WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	sysInitInfo.mCreateWindow		= 1;
	sysInitInfo.mWindowHandle		= NULL;
	sysInitInfo.mHandleWindowMessages = 1;

	// Initialize the system
	if (0 != AESysInit(&sysInitInfo))
		return 1;


	GameStateMgrInit(GS_ASTEROIDS);
	GSM_MainLoop();
	
	// free the system
	AESysExit();

	return 1;
}

// ---------------------------------------------------------------------------