﻿// ---------------------------------------------------------------------------
// Project Name		:	Asteroid Game
// File Name		:	GameState_Play.c
// Author			:	Antoine Abi Chacra
// Creation Date	:	2008/01/31
// Purpose			:	implementation of the 'play' game state
// History			:
// - 2015/12/10		:	Implemented C style component based architecture 
// ---------------------------------------------------------------------------

#include "main.h"
#include "Matrix2D.h"
#include "Vector2D.h"
#include "Math2D.h"

// ---------------------------------------------------------------------------
// Defines

#define SHAPE_NUM_MAX				32					// The total number of different vertex buffer (Shape)
#define GAME_OBJ_INST_NUM_MAX		2048				// The total number of different game object instances


// Feel free to change these values in ordet to make the game more fun
#define SHIP_INITIAL_NUM			3					// Initial number of ship lives
#define SHIP_SIZE					25.0f				// Ship size
#define SHIP_ACCEL_FORWARD			75.0f				// Ship forward acceleration (in m/s^2)
#define SHIP_ACCEL_BACKWARD			-100.0f				// Ship backward acceleration (in m/s^2)
#define SHIP_ROT_SPEED				(2.0f * PI)			// Ship rotation speed (radian/second)
#define HOMING_MISSILE_ROT_SPEED	(PI / 2.0f)			// Homing missile rotation speed (radian/second)
#define BULLET_SPEED				150.0f				// Bullet speed (m/s)

// ---------------------------------------------------------------------------
#define FRICTION	0.99f
#define ASTEROID_SHIP_SCALE		4.f  //Asteroid is 4x larger than ship -- not really but eh
#define ASTEROID_SPEED				50.f
#define BULLET_SIZE		5.f
#define ASTEROID_SIZE	50.f
#define MISSILE_WIDTH	10.f
#define MISSILE_HEIGHT  5.f
#define MISSILE_SPEED	75.f
enum OBJECT_TYPE
{
	// list of game object types
	OBJECT_TYPE_SHIP = 0,
	OBJECT_TYPE_BULLET,
	OBJECT_TYPE_ASTEROID,
	OBJECT_TYPE_HOMING_MISSILE,

	OBJECT_TYPE_NUM
};

// ---------------------------------------------------------------------------
// object mFlag definition

#define FLAG_ACTIVE		0x00000001

// ---------------------------------------------------------------------------
// Struct/Class definitions

typedef struct GameObjectInstance GameObjectInstance;			// Forward declaration needed, since components need to point to their owner "GameObjectInstance"

// ---------------------------------------------------------------------------

typedef struct 
{
	unsigned long			mType;				// Object type (Ship, bullet, etc..)
	AEGfxVertexList*		mpMesh;				// This will hold the triangles which will form the shape of the object

}Shape;

// ---------------------------------------------------------------------------

typedef struct
{
	Shape *mpShape;

	GameObjectInstance *	mpOwner;			// This component's owner
}Component_Sprite;

// ---------------------------------------------------------------------------

typedef struct
{
	Vector2D					mPosition;			// Current position
	float					mAngle;				// Current angle
	float					mScaleX;			// Current X scaling value
	float					mScaleY;			// Current Y scaling value

	Matrix2D					mTransform;			// Object transformation matrix: Each frame, calculate the object instance's transformation matrix and save it here

	GameObjectInstance *	mpOwner;			// This component's owner
}Component_Transform;

// ---------------------------------------------------------------------------

typedef struct
{
	Vector2D					mVelocity;			// Current velocity

	GameObjectInstance *	mpOwner;			// This component's owner
}Component_Physics;

// ---------------------------------------------------------------------------

typedef struct
{
	GameObjectInstance *		mpTarget;		// Target, used by the homing missile

	GameObjectInstance *		mpOwner;		// This component's owner
}Component_Target;

// ---------------------------------------------------------------------------


//Game object instance structure
struct GameObjectInstance
{
	unsigned long				mFlag;						// Bit mFlag, used to indicate if the object instance is active or not

	Component_Sprite			*mpComponent_Sprite;		// Sprite component
	Component_Transform			*mpComponent_Transform;		// Transform component
	Component_Physics			*mpComponent_Physics;		// Physics component
	Component_Target			*mpComponent_Target;		// Target component, used by the homing missile
};

// ---------------------------------------------------------------------------
// Static variables

// List of original vertex buffers
static Shape				sgShapes[SHAPE_NUM_MAX];									// Each element in this array represents a unique shape 
static unsigned long		sgShapeNum;													// The number of defined shapes

// list of object instances
static GameObjectInstance		sgGameObjectInstanceList[GAME_OBJ_INST_NUM_MAX];		// Each element in this array represents a unique game object instance
static unsigned long			sgGameObjectInstanceNum;								// The number of active game object instances

// pointer ot the ship object
static GameObjectInstance*		sgpShip;												// Pointer to the "Ship" game object instance
static Vector2D				sgpShipStartPos;				//Pointer to ship's initial position
static Vector2D				sgpShipStartPhys;				//Pointer to ship's starting physics stuff

// number of ship available (lives 0 = game over)
static long						sgShipLives;											// The number of lives left

// the score = number of asteroid destroyed
static unsigned long			sgScore;												// Current score

// ---------------------------------------------------------------------------

// functions to create/destroy a game object instance
static GameObjectInstance*			GameObjectInstanceCreate(unsigned int ObjectType);			// From OBJECT_TYPE enum
static void							GameObjectInstanceDestroy(GameObjectInstance* pInst);

// ---------------------------------------------------------------------------

// Functions to add/remove components
static void AddComponent_Transform(GameObjectInstance *pInst, Vector2D *pPosition, float Angle, float ScaleX, float ScaleY);
static void AddComponent_Sprite(GameObjectInstance *pInst, unsigned int ShapeType);
static void AddComponent_Physics(GameObjectInstance *pInst, Vector2D *pVelocity);
static void AddComponent_Target(GameObjectInstance *pInst, GameObjectInstance *pTarget);

static void RemoveComponent_Transform(GameObjectInstance *pInst);
static void RemoveComponent_Sprite(GameObjectInstance *pInst);
static void RemoveComponent_Physics(GameObjectInstance *pInst);
static void RemoveComponent_Target(GameObjectInstance *pInst);

// ---------------------------------------------------------------------------

// "Load" function of this state
void GameStateAsteroidsLoad(void)
{
	Shape* pShape = NULL;

	// Zero the shapes array
	memset(sgShapes, 0, sizeof(Shape) * SHAPE_NUM_MAX);
	// No shapes at this point
	sgShapeNum = 0;

	// The ship object instance hasn't been created yet, so this "sgpShip" pointer is initialized to 0
	sgpShip = 0;



	/// Create the game objects(shapes) : Ships, Bullet, Asteroid and Missile
	// How to:
	// -- Remember to create normalized shapes, which means all the vertices' coordinates should be in the [-0.5;0.5] range. Use the object instances' scale values to resize the shape.
	// -- Call “AEGfxMeshStart” to inform the graphics manager that you are about the start sending triangles.
	// -- Call “AEGfxTriAdd” to add 1 triangle.
	// -- A triangle is formed by 3 counter clockwise vertices (points).
	// -- Create all the points between (-0.5, -0.5) and (0.5, 0.5), and use the object instance's scale to change the size.
	// -- Each point can have its own color.
	// -- The color format is : ARGB, where each 2 hexadecimal digits represent the value of the Alpha, Red, Green and Blue respectively. Note that alpha blending(Transparency) is not implemented.
	// -- Each point can have its own texture coordinate (set them to 0.0f in case you’re not applying a texture).
	// -- An object (Shape) can have multiple triangles.
	// -- Call “AEGfxMeshEnd” to inform the graphics manager that you are done creating a mesh, which will return a pointer to the mesh you just created.

	

	// =====================
	// Create the ship shape
	// =====================

	pShape = sgShapes + sgShapeNum++;
	pShape->mType = OBJECT_TYPE_SHIP;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f,  0.5f, 0xFFFF0000, 0.0f, 0.0f, 
		-0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		 0.5f,  0.0f, 0xFFFFFFFF, 0.0f, 0.0f); 
	pShape->mpMesh = AEGfxMeshEnd();


	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 4:
	// -- Create the bullet shape
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	pShape = sgShapes + sgShapeNum++;
	pShape->mType = OBJECT_TYPE_BULLET;

	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f,
		-0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f);
	AEGfxTriAdd(-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f);
	pShape->mpMesh = AEGfxMeshEnd();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 7:
	// -- Create the asteroid shape
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Same as bullet, just different scale?
	pShape = sgShapes + sgShapeNum++;
	pShape->mType = OBJECT_TYPE_ASTEROID;

	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, 0.5f, 0xFFFFFF00, 0.0f, 0.0f,
		-0.5f, -0.5f, 0xFFFFFF00, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFF00, 0.0f, 0.0f);
	AEGfxTriAdd(-0.5f, 0.5f, 0xFFFFFF00, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFFFF00, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFF00, 0.0f, 0.0f);
	pShape->mpMesh = AEGfxMeshEnd();


	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 10:
	// -- Create the homing missile shape
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Same as bullet, just different scale?
	pShape = sgShapes + sgShapeNum++;
	pShape->mType = OBJECT_TYPE_HOMING_MISSILE;

	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pShape->mpMesh = AEGfxMeshEnd();

}

// ---------------------------------------------------------------------------

// "Initialize" function of this state
void GameStateAsteroidsInit(void)
{
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	// zero the game object instance array
	memset(sgGameObjectInstanceList, 0, sizeof(GameObjectInstance)* GAME_OBJ_INST_NUM_MAX);
	// No game object instances (sprites) at this point
	sgGameObjectInstanceNum = 0;

	// create the main ship
	sgpShip = GameObjectInstanceCreate(OBJECT_TYPE_SHIP);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 8:
	// -- Create at least 3 asteroid instances, each with a different size, 
	//    using the "GameObjInstCreate" function
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////


		GameObjectInstance* p = GameObjectInstanceCreate(OBJECT_TYPE_ASTEROID);
		Vector2DSet(&(p->mpComponent_Transform->mPosition), 75, 321);
		Vector2DSet(&(p->mpComponent_Physics->mVelocity), 60, -45);
		p->mpComponent_Transform->mScaleX *= 3;
		p->mpComponent_Transform->mScaleY *= 3;


		 p = GameObjectInstanceCreate(OBJECT_TYPE_ASTEROID);
		Vector2DSet(&(p->mpComponent_Transform->mPosition), -75, 75);
		Vector2DSet(&(p->mpComponent_Physics->mVelocity), -30, 20);
		p->mpComponent_Transform->mScaleX *= 2;
		p->mpComponent_Transform->mScaleY *= 2;


		p = GameObjectInstanceCreate(OBJECT_TYPE_ASTEROID);
		Vector2DSet(&(p->mpComponent_Transform->mPosition),200, 10);
		Vector2DSet(&(p->mpComponent_Physics->mVelocity),-10,22 );

		p = NULL;
	



	// reset the score and the number of ship
	sgScore			= 0;
	sgShipLives		= SHIP_INITIAL_NUM;
}

// ---------------------------------------------------------------------------

// "Update" function of this state
void GameStateAsteroidsUpdate(void)
{
	unsigned long i;
	float winMaxX, winMaxY, winMinX, winMinY;
	double frameTime;

	// ==========================================================================================
	// Getting the window's world edges (These changes whenever the camera moves or zooms in/out)
	// ==========================================================================================
	winMaxX = AEGfxGetWinMaxX();
	winMaxY = AEGfxGetWinMaxY();
	winMinX = AEGfxGetWinMinX();
	winMinY = AEGfxGetWinMinY();


	// ======================
	// Getting the frame time
	// ======================

	frameTime = AEFrameRateControllerGetFrameTime();

	// =========================
	// Update according to input
	// =========================

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 3:
	// -- Compute the forward/backward acceleration of the ship when Up/Down are pressed
	// -- Use the acceleration to update the velocity of the ship
	// -- Limit the maximum velocity of the ship
	// -- IMPORTANT: The current input code moves the ship by simply adjusting its position
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	if (AEInputCheckCurr(VK_UP))
	{
		Vector2D accel;
		Vector2DSet(&accel, cosf(sgpShip->mpComponent_Transform->mAngle), sinf(sgpShip->mpComponent_Transform->mAngle));
		Vector2DScale(&accel, &accel, SHIP_ACCEL_FORWARD);

		Vector2D curVel;
		Vector2DSet(&curVel, sgpShip->mpComponent_Physics->mVelocity.x, sgpShip->mpComponent_Physics->mVelocity.y);
		Vector2DScaleAdd(&(sgpShip->mpComponent_Physics->mVelocity), &accel, &curVel, frameTime);
		Vector2DScale(&(sgpShip->mpComponent_Physics->mVelocity), &(sgpShip->mpComponent_Physics->mVelocity), FRICTION);
		//Vector2DScale(&(sgpShip->mpComponent_Physics->mVelocity), &(sgpShip->mpComponent_Physics->mVelocity), FRICTION);
		//Vector2DAdd(&sgpShip->mpComponent_Transform->mPosition, &sgpShip->mpComponent_Transform->mPosition, &added);
	}

	if (AEInputCheckCurr(VK_DOWN))
	{
		Vector2D accel;
		Vector2DSet(&accel, cosf(sgpShip->mpComponent_Transform->mAngle), sinf(sgpShip->mpComponent_Transform->mAngle));
		Vector2DScale(&accel, &accel, SHIP_ACCEL_BACKWARD);

		Vector2D curVel;
		Vector2DSet(&curVel, sgpShip->mpComponent_Physics->mVelocity.x, sgpShip->mpComponent_Physics->mVelocity.y);
		Vector2DScaleAdd(&(sgpShip->mpComponent_Physics->mVelocity), &accel, &curVel, frameTime);
		Vector2DScale(&(sgpShip->mpComponent_Physics->mVelocity), &(sgpShip->mpComponent_Physics->mVelocity), FRICTION);
		//Vector2DScale(&(sgpShip->mpComponent_Physics->mVelocity), &(sgpShip->mpComponent_Physics->mVelocity), FRICTION);
		//Vector2DAdd(&sgpShip->mpComponent_Transform->mPosition, &sgpShip->mpComponent_Transform->mPosition, &added);
	}

	if (AEInputCheckCurr(VK_LEFT))
	{
		sgpShip->mpComponent_Transform->mAngle += SHIP_ROT_SPEED * (float)(frameTime);
		sgpShip->mpComponent_Transform->mAngle = AEWrap(sgpShip->mpComponent_Transform->mAngle, -PI, PI);
	}

	if (AEInputCheckCurr(VK_RIGHT))
	{
		sgpShip->mpComponent_Transform->mAngle -= SHIP_ROT_SPEED * (float)(frameTime);
		sgpShip->mpComponent_Transform->mAngle = AEWrap(sgpShip->mpComponent_Transform->mAngle, -PI, PI);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 5:
	// -- Create a bullet instance when SPACE is triggered, using the "GameObjInstCreate" function
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	if (AEInputCheckTriggered(VK_SPACE))
	{
		//Double check this
		GameObjectInstance* t;
		t = (GameObjectInstanceCreate(OBJECT_TYPE_BULLET));
		Vector2DSet(&(t->mpComponent_Physics->mVelocity), BULLET_SPEED * cosf(sgpShip->mpComponent_Transform->mAngle), BULLET_SPEED * sinf(sgpShip->mpComponent_Transform->mAngle));
		t = NULL;

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 11:
	// -- Create a homing missile instance when M is triggered
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	if (AEInputCheckTriggered('M'))
	{
		//GameObjectInstanceCreate(OBJECT_TYPE_HOMING_MISSILE);
	//	Vector2DSet(&(GameObjectInstanceCreate(OBJECT_TYPE_HOMING_MISSILE)->mpComponent_Physics->mVelocity), sgpShip->mpComponent_Physics->mVelocity.x, sgpShip->mpComponent_Physics->mVelocity.y);

		GameObjectInstance* t;
		t = (GameObjectInstanceCreate(OBJECT_TYPE_HOMING_MISSILE));
		Vector2DSet(&(t->mpComponent_Physics->mVelocity), MISSILE_SPEED * cosf(sgpShip->mpComponent_Transform->mAngle), MISSILE_SPEED * sinf(sgpShip->mpComponent_Transform->mAngle));
		
		t = NULL;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 2:
	// Update the positions of all active game object instances
	// -- Positions are updated here (P1 = V1*t + P0)
	// -- If implemented correctly, you will be able to control the ship (basic 2D movement)
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////




	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjectInstance* pInst = sgGameObjectInstanceList + i;

		// skip non-active object
		if ((pInst->mFlag & FLAG_ACTIVE) == 0 || pInst == NULL)
			continue;


		//if (pInst->mFlag == OBJECT_TYPE_SHIP)
	//	{

		//}

		Vector2D curPos;
		curPos.x = pInst->mpComponent_Transform->mPosition.x;
		curPos.y = pInst->mpComponent_Transform->mPosition.y;

		pInst->mpComponent_Transform->mPosition.x = (curPos.x + (pInst->mpComponent_Physics->mVelocity.x) * frameTime);

		pInst->mpComponent_Transform->mPosition.y = (curPos.y + (pInst->mpComponent_Physics->mVelocity.y) * frameTime);

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 6: Specific game object behavior, according to type
	// -- Bullet: Destroy when it's outside the viewport
	// -- Asteroids: If it's outside the viewport, wrap around viewport.
	// -- Homing missile: If it's outside the viewport, wrap around viewport.
	// -- Homing missile: Follow/Acquire target
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjectInstance* pInst = sgGameObjectInstanceList + i;

		// skip non-active object
		if ((pInst->mFlag & FLAG_ACTIVE) == 0)
			continue;

		// check if the object is a ship
		if (pInst->mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_SHIP)
		{
			// warp the ship from one end of the screen to the other
			pInst->mpComponent_Transform->mPosition.x = AEWrap(pInst->mpComponent_Transform->mPosition.x, winMinX - SHIP_SIZE, winMaxX + SHIP_SIZE);
			pInst->mpComponent_Transform->mPosition.y = AEWrap(pInst->mpComponent_Transform->mPosition.y, winMinY - SHIP_SIZE, winMaxY + SHIP_SIZE);
		}

		// Bullet behavior
		else if (pInst->mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_BULLET)
		{

			if (pInst->mpComponent_Transform->mPosition.x > winMaxX || pInst->mpComponent_Transform->mPosition.x < winMinX || pInst->mpComponent_Transform->mPosition.y > winMaxY || pInst->mpComponent_Transform->mPosition.y < winMinY)
			{
				GameObjectInstanceDestroy(pInst);
			}
		}

		// Asteroid behavior
		else if (pInst->mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_ASTEROID)
		{

			pInst->mpComponent_Transform->mPosition.x = AEWrap(pInst->mpComponent_Transform->mPosition.x, winMinX - ASTEROID_SIZE, winMaxX + ASTEROID_SIZE);
			pInst->mpComponent_Transform->mPosition.y = AEWrap(pInst->mpComponent_Transform->mPosition.y, winMinY - ASTEROID_SIZE, winMaxY + ASTEROID_SIZE);
		}

		// Homing missile behavior (Not every game object instance will have this component!)

		else if (pInst->mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_HOMING_MISSILE)
		{
			pInst->mpComponent_Transform->mPosition.x = AEWrap(pInst->mpComponent_Transform->mPosition.x, winMinX - MISSILE_WIDTH, winMaxX + MISSILE_WIDTH);
			pInst->mpComponent_Transform->mPosition.y = AEWrap(pInst->mpComponent_Transform->mPosition.y, winMinY - MISSILE_HEIGHT, winMaxY + MISSILE_HEIGHT);


			if (pInst->mpComponent_Target->mpTarget == NULL  || pInst->mpComponent_Target->mpTarget->mFlag != FLAG_ACTIVE)
			{
				for (int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
				{
					if (sgGameObjectInstanceList[i].mFlag == FLAG_ACTIVE && sgGameObjectInstanceList[i].mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_ASTEROID)
					{
						pInst->mpComponent_Target->mpTarget = &sgGameObjectInstanceList[i];
						i = GAME_OBJ_INST_NUM_MAX;
					}
				}
			}

			//Homing logic goes here
			if (pInst->mpComponent_Target->mpTarget != NULL && pInst->mpComponent_Target->mpTarget->mFlag == FLAG_ACTIVE)
			{
				Vector2D mVel, normal, asteroidVec;

				Vector2DSet(&mVel, pInst->mpComponent_Physics->mVelocity.x, pInst->mpComponent_Physics->mVelocity.y);
				Vector2DSet(&normal, -1 * mVel.y, mVel.x);
				Vector2DSet(&asteroidVec, (pInst->mpComponent_Target->mpTarget->mpComponent_Transform->mPosition.x) - (pInst->mpComponent_Transform->mPosition.x), (pInst->mpComponent_Target->mpTarget->mpComponent_Transform->mPosition.y) - (pInst->mpComponent_Transform->mPosition.y));

				float angle = (mVel.x * asteroidVec.x + mVel.y * asteroidVec.y) / (Vector2DLength(&mVel) * Vector2DLength(&asteroidVec));  //May need to turn to radians, check disssss
				float a = min(HOMING_MISSILE_ROT_SPEED * frameTime, acosf(angle ));

				if (normal.x * asteroidVec.x + normal.y * asteroidVec.y < 0)
				{
					a = -a;
				}

			float curAngle =	pInst->mpComponent_Transform->mAngle + a;
				pInst->mpComponent_Transform->mAngle += a;
				//float curAngle = pInst->mpComponent_Transform->mAngle +a;
				Vector2DSet(&(pInst->mpComponent_Physics->mVelocity), cosf(curAngle), sinf(curAngle));
				Vector2DNormalize(&(pInst->mpComponent_Physics->mVelocity), &(pInst->mpComponent_Physics->mVelocity));
				Vector2DScale(&(pInst->mpComponent_Physics->mVelocity), &(pInst->mpComponent_Physics->mVelocity), MISSILE_SPEED, MISSILE_SPEED);
			}
		}




	}


	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 9: Check for collision
	// Important: Take the object instance's scale values into consideration when checking for collision.
	// -- Asteroid - Bullet: Rectangle to Point check. If they collide, destroy both.
	// -- Asteroid - Ship: Rectangle to Rectangle check. If they collide, destroy the asteroid, 
	//    reset the ship position to the center of the screen.
	// -- Asteroid - Homing Missile: Rectangle to Rectangle check.
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	for each object instance: oi1
		if oi1 is not active
			skip

		if oi1 is an asteroid
			for each object instance oi2
				if(oi2 is not active or oi2 is an asteroid)
					skip

				if(oi2 is the ship)
					Check for collision between the ship and the asteroid
					Update game behavior accordingly
					Update "Object instances array"
				else
				if(oi2 is a bullet)
					Check for collision between the bullet and the asteroid
					Update game behavior accordingly
					Update "Object instances array"
				else
				if(oi2 is a missle)
					Check for collision between the missile and the asteroid
					Update game behavior accordingly
					Update "Object instances array"
	*/


	for (int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
	

		if ( sgGameObjectInstanceList[i].mFlag == FLAG_ACTIVE && sgGameObjectInstanceList[i].mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_ASTEROID)
		{
			for (int j = 0; j < GAME_OBJ_INST_NUM_MAX; j++)
			{
				if (sgGameObjectInstanceList[i].mFlag != FLAG_ACTIVE)
				{
					j = GAME_OBJ_INST_NUM_MAX;
				}
				else{
					if (sgGameObjectInstanceList[j].mFlag == FLAG_ACTIVE)
					{
						if (sgGameObjectInstanceList[j].mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_SHIP)
						{
							if (1 == StaticRectToStaticRect(&(sgGameObjectInstanceList[i].mpComponent_Transform->mPosition), sgGameObjectInstanceList[i].mpComponent_Transform->mScaleX, sgGameObjectInstanceList[i].mpComponent_Transform->mScaleY, &(sgGameObjectInstanceList[j].mpComponent_Transform->mPosition), sgGameObjectInstanceList[j].mpComponent_Transform->mScaleX, sgGameObjectInstanceList[j].mpComponent_Transform->mScaleY))
							{
								GameObjectInstanceDestroy(&(sgGameObjectInstanceList[i]));
								//GameObjectInstanceDestroy(&(sgGameObjectInstanceList[j]));
								//sgpShip = GameObjectInstanceCreate(OBJECT_TYPE_SHIP);


								Vector2DSet(&sgpShip->mpComponent_Transform->mPosition, sgpShipStartPos.x, sgpShipStartPos.y);
								Vector2DSet(&sgpShip->mpComponent_Physics->mVelocity, sgpShipStartPhys.x, sgpShipStartPhys.y);
								//sgpShip->mpComponent_Transform = sgpShipStartPos;
								//sgpShip->mpComponent_Physics = sgpShipStartPhys;
							}
						}


						else if (sgGameObjectInstanceList[j].mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_BULLET)
						{
							if (1 == StaticPointToStaticRect(&(sgGameObjectInstanceList[j].mpComponent_Transform->mPosition), &(sgGameObjectInstanceList[i].mpComponent_Transform->mPosition), sgGameObjectInstanceList[i].mpComponent_Transform->mScaleX, sgGameObjectInstanceList[i].mpComponent_Transform->mScaleY))
							{
								GameObjectInstanceDestroy(&(sgGameObjectInstanceList[i]));
								GameObjectInstanceDestroy(&(sgGameObjectInstanceList[j]));
							}
						}


						else if (sgGameObjectInstanceList[j].mpComponent_Sprite->mpShape->mType == OBJECT_TYPE_HOMING_MISSILE)
						{
							if (1 == StaticRectToStaticRect(&(sgGameObjectInstanceList[i].mpComponent_Transform->mPosition), sgGameObjectInstanceList[i].mpComponent_Transform->mScaleX, sgGameObjectInstanceList[i].mpComponent_Transform->mScaleY, &(sgGameObjectInstanceList[j].mpComponent_Transform->mPosition), sgGameObjectInstanceList[j].mpComponent_Transform->mScaleX, sgGameObjectInstanceList[j].mpComponent_Transform->mScaleY))
							{
								GameObjectInstanceDestroy(&(sgGameObjectInstanceList[i]));
								GameObjectInstanceDestroy(&(sgGameObjectInstanceList[j]));

							}
						}
					}
				}
			}
		}
	}


	// =====================================
	// calculate the matrix for all objects
	// =====================================

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		Matrix2D		 trans, rotate, scale;
		GameObjectInstance* pInst = sgGameObjectInstanceList + i;
		
		// skip non-active object
		if(pInst == NULL || (pInst->mFlag & FLAG_ACTIVE) == 0  )
			continue;


		/////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////
		// TO DO 1:
		// -- Build the transformation matrix of each active game object instance
		// -- After you implement this step, you should see the player's ship
		// -- Reminder: Scale should be applied first, then rotation, then translation.
		/////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////

		Matrix2DScale(&scale, pInst->mpComponent_Transform->mScaleX, pInst->mpComponent_Transform->mScaleY);
		Matrix2DRotRad(&rotate, pInst->mpComponent_Transform->mAngle);
		Matrix2DTranslate(&trans, pInst->mpComponent_Transform->mPosition.x, pInst->mpComponent_Transform->mPosition.y);

		Matrix2DIdentity(&(pInst->mpComponent_Transform->mTransform));
		Matrix2DConcat(&(pInst->mpComponent_Transform->mTransform), &trans, &rotate);
		Matrix2DConcat(&(pInst->mpComponent_Transform->mTransform), &(pInst->mpComponent_Transform->mTransform), &scale);



		// Compute the scaling matrix
		// Compute the rotation matrix 
		// Compute the translation matrix
		// Concatenate the 3 matrix in the correct order in the object instance's transform component's "mTransform" matrix
	}
}
// ---------------------------------------------------------------------------

void GameStateAsteroidsDraw(void)
{
	unsigned long i;
	double frameTime;


	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxTextureSet(NULL, 0, 0);
	AEGfxSetTintColor(1.0f, 1.0f, 1.0f, 1.0f);

	// draw all object instances in the list

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjectInstance* pInst = sgGameObjectInstanceList + i;

		// skip non-active object
		if ((pInst->mFlag & FLAG_ACTIVE) == 0  || pInst==NULL)
			continue;
		
		// Already implemented. Explanation:
		// Step 1 & 2 are done outside the for loop (AEGfxSetRenderMode, AEGfxTextureSet, AEGfxSetTintColor) since all our objects share the same material.
		// If you want to have objects with difference materials (Some with textures, some without, some with transparency etc...)
		// then you'll need to move those functions calls inside the for loop
		// 1 - Set Render Mode (Color or texture)
		// 2 - Set all needed parameters (Color blend, textures, etc..)
		// 3 - Set the current object instance's mTransform matrix using "AEGfxSetTransform"
		// 4 - Draw the shape used by the current object instance using "AEGfxMeshDraw"

		AEGfxSetTransform(pInst->mpComponent_Transform->mTransform.m);
		AEGfxMeshDraw(pInst->mpComponent_Sprite->mpShape->mpMesh, AE_GFX_MDM_TRIANGLES);
	}
}

// ---------------------------------------------------------------------------

void GameStateAsteroidsFree(void)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 12:
	//  -- Destroy all the active game object instances, using the “GameObjInstanceDestroy” function.
	//  -- Reset the number of active game objects instances
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////

	for (int i = 0; sgGameObjectInstanceNum > 0 && i < GAME_OBJ_INST_NUM_MAX &&  sgGameObjectInstanceList[i].mFlag == FLAG_ACTIVE ; i++)
	{
		GameObjectInstanceDestroy(&(sgGameObjectInstanceList[i]));
	}



}

// ---------------------------------------------------------------------------

void GameStateAsteroidsUnload(void)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// TO DO 13:
	//  -- Destroy all the shapes, using the “AEGfxMeshFree” function.
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < sgShapeNum; i++)  //CHECK - might be <= instead of <
	{
		AEGfxMeshFree(sgShapes[i].mpMesh);
	}

}

// ---------------------------------------------------------------------------

GameObjectInstance* GameObjectInstanceCreate(unsigned int ObjectType)			// From OBJECT_TYPE enum)
{
	unsigned long i;
	
	// loop through the object instance list to find a non-used object instance
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjectInstance* pInst = sgGameObjectInstanceList + i;

		// Check if current instance is not used
		if (pInst->mFlag == 0)
		{
			// It is not used => use it to create the new instance

			// Active the game object instance
			pInst->mFlag = FLAG_ACTIVE;

			pInst->mpComponent_Transform = 0;
			pInst->mpComponent_Sprite = 0;
			pInst->mpComponent_Physics = 0;
			pInst->mpComponent_Target = 0;

			// Add the components, based on the object type
			switch (ObjectType)
			{
			case OBJECT_TYPE_SHIP:
				AddComponent_Sprite(pInst, OBJECT_TYPE_SHIP);
				AddComponent_Transform(pInst, 0, 0.0f, SHIP_SIZE, SHIP_SIZE);   //Initial scale is 1, setting it to predefined SHIP_SIZE
				AddComponent_Physics(pInst, 0);
				Vector2DSet(&sgpShipStartPos, pInst->mpComponent_Transform->mPosition.x, pInst->mpComponent_Transform->mPosition.y);
				Vector2DSet(&sgpShipStartPhys, pInst->mpComponent_Physics->mVelocity.x, pInst->mpComponent_Physics->mVelocity.y);
				break;
				
			case OBJECT_TYPE_BULLET:
				AddComponent_Sprite(pInst, OBJECT_TYPE_BULLET);
				AddComponent_Transform(pInst, &(sgpShip->mpComponent_Transform->mPosition), 0.0f, BULLET_SIZE, BULLET_SIZE);
				AddComponent_Physics(pInst, 0);
				break;

			case OBJECT_TYPE_ASTEROID:
				AddComponent_Sprite(pInst, OBJECT_TYPE_ASTEROID);
				AddComponent_Transform(pInst, 0, 0.0f,ASTEROID_SIZE, ASTEROID_SIZE);
				AddComponent_Physics(pInst, 0);
				break;

			case OBJECT_TYPE_HOMING_MISSILE:
				AddComponent_Sprite(pInst, OBJECT_TYPE_HOMING_MISSILE);
				AddComponent_Transform(pInst, &(sgpShip->mpComponent_Transform->mPosition), (sgpShip->mpComponent_Transform->mAngle),MISSILE_WIDTH, MISSILE_HEIGHT);
				AddComponent_Physics(pInst, 0);
				AddComponent_Target(pInst, 0);
				break;
			}

			++sgGameObjectInstanceNum;

			// return the newly created instance
			return pInst;
		}
	}

	// Cannot find empty slot => return 0
	return 0;
}

// ---------------------------------------------------------------------------

void GameObjectInstanceDestroy(GameObjectInstance* pInst)
{
	// if instance is destroyed before, just return
	if (pInst->mFlag == 0)
		return;

	// Zero out the mFlag
	pInst->mFlag = 0;

	RemoveComponent_Transform(pInst);
	RemoveComponent_Sprite(pInst);
	RemoveComponent_Physics(pInst);
	RemoveComponent_Target(pInst);

	--sgGameObjectInstanceNum;
}

// ---------------------------------------------------------------------------

void AddComponent_Transform(GameObjectInstance *pInst, Vector2D *pPosition, float Angle, float ScaleX, float ScaleY)
{
	if (0 != pInst)
	{
		if (0 == pInst->mpComponent_Transform)
		{
			pInst->mpComponent_Transform = (Component_Transform *)calloc(1, sizeof(Component_Transform));
		}

		Vector2D zeroVec2;
		Vector2DZero(&zeroVec2);

		pInst->mpComponent_Transform->mScaleX = ScaleX;
		pInst->mpComponent_Transform->mScaleY = ScaleY;
		pInst->mpComponent_Transform->mPosition = pPosition ? *pPosition : zeroVec2;;
		pInst->mpComponent_Transform->mAngle = Angle;
		pInst->mpComponent_Transform->mpOwner = pInst;
	}
}

// ---------------------------------------------------------------------------

void AddComponent_Sprite(GameObjectInstance *pInst, unsigned int ShapeType)
{
	if (0 != pInst)
	{
		if (0 == pInst->mpComponent_Sprite)
		{
			pInst->mpComponent_Sprite = (Component_Sprite *)calloc(1, sizeof(Component_Sprite));
		}
	
		pInst->mpComponent_Sprite->mpShape = sgShapes + ShapeType;
		pInst->mpComponent_Sprite->mpOwner = pInst;
	}
}

// ---------------------------------------------------------------------------

void AddComponent_Physics(GameObjectInstance *pInst, Vector2D *pVelocity)
{
	if (0 != pInst)
	{
		if (0 == pInst->mpComponent_Physics)
		{
			pInst->mpComponent_Physics = (Component_Physics *)calloc(1, sizeof(Component_Physics));
		}

		Vector2D zeroVec2;
		Vector2DZero(&zeroVec2);

		pInst->mpComponent_Physics->mVelocity = pVelocity ? *pVelocity : zeroVec2;
		pInst->mpComponent_Physics->mpOwner = pInst;
	}
}

// ---------------------------------------------------------------------------

void AddComponent_Target(GameObjectInstance *pInst, GameObjectInstance *pTarget)
{
	if (0 != pInst)
	{
		if (0 == pInst->mpComponent_Target)
		{
			pInst->mpComponent_Target = (Component_Target *)calloc(1, sizeof(Component_Target));
		}

		pInst->mpComponent_Target->mpTarget = pTarget;
		pInst->mpComponent_Target->mpOwner = pInst;
	}
}

// ---------------------------------------------------------------------------

void RemoveComponent_Transform(GameObjectInstance *pInst)
{
	if (0 != pInst)
	{
		if (0 != pInst->mpComponent_Transform)
		{
			free(pInst->mpComponent_Transform);
			pInst->mpComponent_Transform = 0;
		}
	}
}

// ---------------------------------------------------------------------------

void RemoveComponent_Sprite(GameObjectInstance *pInst)
{
	if (0 != pInst)
	{
		if (0 != pInst->mpComponent_Sprite)
		{
			free(pInst->mpComponent_Sprite);
			pInst->mpComponent_Sprite = 0;
		}
	}
}

// ---------------------------------------------------------------------------

void RemoveComponent_Physics(GameObjectInstance *pInst)
{
	if (0 != pInst)
	{
		if (0 != pInst->mpComponent_Physics)
		{
			free(pInst->mpComponent_Physics);
			pInst->mpComponent_Physics = 0;
		}
	}
}

// ---------------------------------------------------------------------------

void RemoveComponent_Target(GameObjectInstance *pInst)
{
	if (0 != pInst)
	{
		if (0 != pInst->mpComponent_Target)
		{
			free(pInst->mpComponent_Target);
			pInst->mpComponent_Target = 0;
		}
	}
}
