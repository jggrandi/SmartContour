#include <ErrorManager.h>

#include "IMedicalImage.h"

/**
*/
IMedicalImage::IMedicalImage(void)
{
	Create();
}

/**
*/
IMedicalImage::~IMedicalImage(void)
{
}

/**
*/
const std::string &IMedicalImage::GetSeriesInstanceUID(void) const
{
	return m_seriesInstanceUID;
}

/**
*/
int IMedicalImage::GetSeriesNumber(void) const
{
	return m_seriesNumber;
}

/**
*/
int IMedicalImage::GetTimeHour(void) const
{
	return m_acquisitionTimeHour;
}

/**
*/
int IMedicalImage::GetTimeMinute(void) const
{
	return m_acquisitionTimeMinute;
}

/**
*/
int IMedicalImage::GetTimeSecond(void) const
{
	return m_acquisitionTimeSecond;
}

/**
*/
int IMedicalImage::GetTimeMilisecond(void) const
{
	return m_acquisitionTimeMilisecond;
}

/**
*/
int IMedicalImage::GetNumber(void) const
{
	return m_imageNumber;
}

/**
*/
const float *IMedicalImage::GetPixelSpacing(void) const
{
	return m_pixelSpacing;
}

/**
*/
float IMedicalImage::GetSliceLocation(void) const
{
	return m_sliceLocation;
}

/**
*/
const float *IMedicalImage::GetImagePosition(void) const
{
	return m_imagePosition;
}

/**
*/
const float *IMedicalImage::GetFirstImageOrientationAxis(void) const
{
	return m_imageOrientation[0];
}

/**
*/
const float *IMedicalImage::GetSecondImageOrientationAxis(void) const
{
	return m_imageOrientation[1];
}

/**
*/
void IMedicalImage::Create(void)
{
	IImage::Create();

	m_acquisitionDateYear = 0;
	m_acquisitionDateMonth = 0;
	m_acquisitionDateDay = 0;
	m_acquisitionTimeHour = 0;
	m_acquisitionTimeMinute = 0;
	m_acquisitionTimeSecond = 0;
	m_acquisitionTimeMilisecond = 0;
	
	m_patientName.clear();
	m_patientSex.clear();
	m_patientAge = 0;

	m_sliceThickness = 0.0f;

	m_seriesInstanceUID.clear();
	m_seriesNumber = 0;
	m_imageNumber = 0;

	m_imagePosition[0] = 0.0f;
	m_imagePosition[1] = 0.0f;
	m_imagePosition[2] = 0.0f;
	
	m_imageOrientation[0][0] = 0.0f;
	m_imageOrientation[0][1] = 0.0f;
	m_imageOrientation[0][2] = 0.0f;
	m_imageOrientation[1][0] = 0.0f;
	m_imageOrientation[1][1] = 0.0f;
	m_imageOrientation[1][2] = 0.0f;

	m_sliceLocation = 0.0f;

	m_height = 0;
	m_width = 0;
	m_pixelSpacing[0] = 0.0f;
	m_pixelSpacing[1] = 0.0f;
	m_bitsAllocated = 0;
	m_bitsStored = 0;
	m_highBit = 0;
	
	m_windowCenter = 400;
	m_windowWidth = 2000;
}

