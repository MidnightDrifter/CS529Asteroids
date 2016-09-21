/* Start Header -------------------------------------------------------
Copyright Matrix2D.c
Purpose:  Implementation of basic matrix functionality
Language:  C
Platform: Windows OS, VS2015 Express for Win. Desktop
Project: sean.higgins CS529_Matrix2D.c_1
Author: Sean Higgins, sean.higgins
Creation date: 9-14-2016
- End Header --------------------------------------------------------*/


#include "Matrix2D.h"


/*
This function sets the matrix Result to the identity matrix
*/
void Matrix2DIdentity(Matrix2D *pResult)
{
	pResult->m[0][0] = 1.f;
	pResult->m[0][1] = 0.f;
	pResult->m[0][2] = 0.f;
	pResult->m[1][0] = 0.f;
	pResult->m[1][1] = 1.f;
	pResult->m[1][2] = 0.f;
	pResult->m[2][0] = 0.f;
	pResult->m[2][1] = 0.f;
	pResult->m[2][2] = 1.f;
}

void Matrix2DZero(Matrix2D *pResult)  //SH added fn. -- zeroes out a matrix
{
	pResult->m[0][0] = 0.f;
	pResult->m[0][1] = 0.f;
	pResult->m[0][2] = 0.f;
	pResult->m[1][0] = 0.f;
	pResult->m[1][1] = 0.f;
	pResult->m[1][2] = 0.f;
	pResult->m[2][0] = 0.f;
	pResult->m[2][1] = 0.f;
	pResult->m[2][2] = 0.f;
}

// ---------------------------------------------------------------------------

/*
This functions calculated the transpose matrix of Mtx and saves it in Result
*/
void Matrix2DTranspose(Matrix2D *pResult, Matrix2D *pMtx)
{
	Matrix2D t;
	for(int i=0;i<3;i++)
	{
		for (int j = 0; j < 3; j++)
		{
			t.m[i][j] = pMtx->m[j][i];
		}
	}

	*pResult = t;
}

// ---------------------------------------------------------------------------

/*
This function multiplies Mtx0 with Mtx1 and saves the result in Result
Result = Mtx0*Mtx1
*/
void Matrix2DConcat(Matrix2D *pResult, Matrix2D *pMtx0, Matrix2D *pMtx1)
{
	Matrix2D t;
	Matrix2DZero(&t);
	for(int i=0;i<3;i++)
	{
		float sum = 0.f;
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				t.m[i][j] += pMtx0->m[i][k] * pMtx1->m[k][j];
			}
		}
	}

	*pResult = t;
}

// ---------------------------------------------------------------------------

/*
This function creates a translation matrix from x & y and saves it in Result
*/
void Matrix2DTranslate(Matrix2D *pResult, float x, float y)
{
	pResult->m[0][0] = 1.f;
	pResult->m[0][1] = 0.f;
	pResult->m[0][2] = x;
	pResult->m[1][0] = 0.f;
	pResult->m[1][1] = 1.f;
	pResult->m[1][2] = y;
	pResult->m[2][0] = 0.f;
	pResult->m[2][1] = 0.f;
	pResult->m[2][2] = 1.f;
}


// ---------------------------------------------------------------------------

/*
This function creates a scaling matrix from x & y and saves it in Result
*/
void Matrix2DScale(Matrix2D *pResult, float x, float y)
{
	pResult->m[0][0] = x;
	pResult->m[0][1] = 0.f;
	pResult->m[0][2] = 0.f;
	pResult->m[1][0] = 0.f;
	pResult->m[1][1] = y;
	pResult->m[1][2] = 0.f;
	pResult->m[2][0] = 0.f;
	pResult->m[2][1] = 0.f;
	pResult->m[2][2] = 1.f;

}

// ---------------------------------------------------------------------------

/*
This matrix creates a rotation matrix from "Angle" whose value is in degree.
Save the resultant matrix in Result
*/
void Matrix2DRotDeg(Matrix2D *pResult, float Angle)
{

	Matrix2DRotRad(pResult, Angle * 3.14159265358979323846f / 180.f);

}

// ---------------------------------------------------------------------------

/*
This matrix creates a rotation matrix from "Angle" whose value is in radian.
Save the resultant matrix in Result
*/
void Matrix2DRotRad(Matrix2D *pResult, float Angle)
{
	pResult->m[0][0] = cosf(Angle);
	pResult->m[0][1] = -1.f*sinf(Angle);
	pResult->m[0][2] = 0.f;
	pResult->m[1][0] = -1.f*pResult->m[0][1];
	pResult->m[1][1] =  pResult->m[0][0];
	pResult->m[1][2] = 0.f;
	pResult->m[2][0] = 0.f;
	pResult->m[2][1] = 0.f;
	pResult->m[2][2] = 1.f;

}

// ---------------------------------------------------------------------------

/*
This function multiplies the matrix Mtx with the vector Vec and saves the result in Result
Result = Mtx * Vec
*/
void Matrix2DMultVec(Vector2D *pResult, Matrix2D *pMtx, Vector2D *pVec)
{
	Vector2D t;  //Emulating homogenous coord of 1 at the end
	t.x = pMtx->m[0][0] * pVec->x + pMtx->m[0][1] * pVec->y + pMtx->m[0][2];
	t.y = pMtx->m[1][0] * pVec->x + pMtx->m[1][1] * pVec->y + pMtx->m[1][2];

	*pResult = t;

}

// ---------------------------------------------------------------------------
