/* Start Header -------------------------------------------------------
Copyright Math2D.c
Purpose:  Implementation of basic math / collision functionality
Language:  C
Platform: Windows OS, VS2015 Express for Win. Desktop
Project: sean.higgins CS529_Math2D.c_1
Author: Sean Higgins, sean.higgins
Creation date: 9-14-2016
- End Header --------------------------------------------------------*/

#include "Math2D.h"
#include "stdio.h"

/*
This function checks if the point P is colliding with the circle whose
center is "Center" and radius is "Radius"
*/
int StaticPointToStaticCircle(Vector2D *pP, Vector2D *pCenter, float Radius)
{
	if(Vector2DSquareDistance(pP, pCenter) > Radius*Radius)
	{
		return 0;
	}
	return 1;
}


/*
This function checks if the point Pos is colliding with the rectangle
whose center is Rect, width is "Width" and height is Height
*/
int StaticPointToStaticRect(Vector2D *pPos, Vector2D *pRect, float Width, float Height)
{
	if (pPos->x < pRect->x - Width / 2 || pPos->x > pRect->x + Width / 2 || pPos->y < pRect->y - Height / 2 || pPos->y > pRect->y + Height / 2)
	{
		return 0;
	}

	return 1;
}

/*
This function checks for collision between 2 circles.
Circle0: Center is Center0, radius is "Radius0"
Circle1: Center is Center1, radius is "Radius1"
*/
int StaticCircleToStaticCircle(Vector2D *pCenter0, float Radius0, Vector2D *pCenter1, float Radius1)
{
	if (Vector2DSquareDistance(pCenter0, pCenter1) > powf(Radius0 + Radius1, 2))
	{
		return 0;
	}

	return 1;
}

/*
This functions checks if 2 rectangles are colliding
Rectangle0: Center is pRect0, width is "Width0" and height is "Height0"
Rectangle1: Center is pRect1, width is "Width1" and height is "Height1"
*/
int StaticRectToStaticRect(Vector2D *pRect0, float Width0, float Height0, Vector2D *pRect1, float Width1, float Height1)
{
	if (pRect0->x - Width0 / 2 > pRect1->x + Width1 / 2 || pRect1->x - Width1 / 2 > pRect0->x + Width0 / 2 || pRect0->y - Height0 / 2 > pRect1->y + Height1 / 2 || pRect1->y - Height1 / 2 > pRect0->y + Height0 / 2)
	{
		return 0;
	}

	return 1;
}
