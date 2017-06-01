#include <ErrorManager.h>
#include <MyMath.h>

#include "IImage.h"

using namespace std;

/**
*/
IImage::IImage(void)
{
	Create();
}

/**
*/
IImage::~IImage(void)
{
}

/**
*/
void IImage::Create(void)
{
	m_width = 0;
	m_height = 0;
	
	m_imageData.clear();
}

