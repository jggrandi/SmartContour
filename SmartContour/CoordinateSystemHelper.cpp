#include <iostream>

#include <ErrorManager.h>
#include <MyMath.h>
#include <MyGLH.h>

#include "CoordinateSystemHelper.h"

using namespace std;

/**
*/
CCoordinateSystemHelper::CCoordinateSystemHelper(void)
{
	Create();
}

/**
*/
CCoordinateSystemHelper::~CCoordinateSystemHelper(void)
{
}

/**
*/
bool CCoordinateSystemHelper::AttachToCamera(boost::shared_ptr<CPinholeCamera> camera)
{
	if (!camera.get())
	{
		MarkError();

		return false;
	}

	m_attachedCamera = camera;

	return true;
}

/**
*/
bool CCoordinateSystemHelper::ScreenToImage(int x, int y, float *s, float *t)
{
	float inputVertex[3],
		outputVertex[3];

	if (!m_attachedCamera.get())
	{
		MarkError();

		return false;
	}

	inputVertex[0] = float(x);
	inputVertex[1] = float(y);
	inputVertex[2] = 0.0f;

	if (!CMyGLH::UnprojectVertex3(m_attachedCamera->GetViewMatrix(), m_attachedCamera->GetProjectionMatrix(), m_attachedCamera->GetViewport(), inputVertex, outputVertex))
	{
		MarkError();

		return false;
	}

	*s = outputVertex[0];
	*t = outputVertex[1];

	return true;
}
/*
bool CCoordinateSystemHelper::ImageToScreen(float x, float y, int *s, int *t)
{
	float inputVertex[3],
		outputVertex[3];

	if (!m_attachedCamera.get())
	{
		MarkError();

		return false;
	}

	inputVertex[0] = int(x);
	inputVertex[1] = int(y);
	inputVertex[2] = 0.0f;

	if (!CMyGLH::ProjectVertex3(m_attachedCamera->GetViewMatrix(), m_attachedCamera->GetProjectionMatrix(), m_attachedCamera->GetViewport(), inputVertex, outputVertex))
	{
		MarkError();

		return false;
	}

	*s = outputVertex[0];
	*t = outputVertex[1];

	return true;
}*/

/**
*/
void CCoordinateSystemHelper::Create(void)
{
	m_attachedCamera.reset();
}

