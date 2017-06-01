#include <ErrorManager.h>
#include <MyMath.h>

#include "SliceViewer.h"

using namespace std;

/**
*/
CSliceViewer::CSliceViewer(void)
{
	Create();
}

/**
*/
CSliceViewer::~CSliceViewer(void)
{
}

/**
*/
bool CSliceViewer::Initialize(boost::shared_ptr<CSeries> series, IMedicalImage::ImageViewPlaneEnum viewPlane, float sliceLocation, int windowCenter, int windowWidth)
{
	if (!series.get())
	{
		MarkError();

		return false;
	}

	switch (viewPlane) 
	{
	case IMedicalImage::AXIAL_VIEW_PLANE:
		if (!ReadAxialViewPlaneDataFromSeries(series, sliceLocation))
		{
			MarkError();
			
			return false;
		}
		break;
	case IMedicalImage::SAGITTAL_VIEW_PLANE:
		if (!ReadSagittalViewPlaneDataFromSeries(series, sliceLocation))
		{
			MarkError();
			
			return false;
		}
		break;
	case IMedicalImage::CORONAL_VIEW_PLANE:
		if (!ReadCoronalViewPlaneDataFromSeries(series, sliceLocation))
		{
			MarkError();
			
			return false;
		}
		break;
	};
		
	if (!m_imageDataTexture.get())
	{
		m_imageDataTexture.reset(new CTexture);
		
		if (!m_imageDataTexture.get())
		{
			MarkError();
			
			return false;
		}
		
		if (!m_imageDataTexture->Initialize(CTexture::TEXTURE_2D, CTexture::LUMINANCE))
		{
			MarkError();
			
			return false;
		}

		m_imageDataTexture->SetParameter(CTexture::MIN_FILTER, CTexture::LINEAR);
		m_imageDataTexture->SetParameter(CTexture::MAG_FILTER, CTexture::LINEAR);
		m_imageDataTexture->SetParameter(CTexture::WRAP_S, CTexture::CLAMP);
		m_imageDataTexture->SetParameter(CTexture::WRAP_T, CTexture::CLAMP);
	}
	
	if (!m_imageDataTexture->SetImage(m_width, m_height, 0, CTexture::LUMINANCE, CTexture::FLOAT, 0))
	{
		MarkError();

		return false;
	}

	if (!UpdateWindow(windowCenter, windowWidth))
	{
		MarkError();

		return false;
	}

	return true;
}

/**
*/
const char *CSliceViewer::GetName(void) const
{
	return m_sliceName;
}

/**
*/
bool CSliceViewer::UpdateWindow(int windowCenter, int windowWidth)
{
	vector<float> windowedSliceData;
	float *windowedPixel,
		*pixel,
		windowBegin,
		windowEnd;
	int imageSize,
		i;

	if (!m_imageDataTexture.get())
	{
		MarkError();

		return false;
	}

	if (!m_imageDataTexture->IsValid())
	{
		MarkError();

		return false;
	}

	// The use of a negative window width allows an interesting threshold 
	// effect, but it doesn't seems to be correct.
	if (windowWidth < 0)
		windowWidth = 0;

	if (windowCenter < 0)
		windowCenter = 0;

	imageSize = m_imageDataTexture->GetWidth()*m_imageDataTexture->GetHeight();

	if (m_imageData.size() != imageSize)
	{
		MarkError();

		return false;
	}

	windowedSliceData.assign(imageSize, 0.0f);

	windowedPixel = &windowedSliceData[0];

	pixel = &m_imageData[0];

	windowBegin = (float)windowCenter - 0.5f*windowWidth;
	windowEnd = windowBegin + windowWidth;

	for (i=0; i<imageSize; ++i)
	{
		if (*pixel > windowEnd)
			*windowedPixel = 1.0f;
		else if (*pixel > windowBegin)
			*windowedPixel = (*pixel - windowBegin)/windowWidth;

		windowedPixel++;
		pixel++;
	}

	if (!m_imageDataTexture->SetSubImage(0, 0, m_imageDataTexture->GetWidth(), m_imageDataTexture->GetHeight(), CTexture::LUMINANCE, CTexture::FLOAT, &windowedSliceData[0]))
	{
		MarkError();

		return false;
	}

	m_windowCenter = windowCenter;
	m_windowWidth = windowWidth;

	return true;
}

/**
*/
IMedicalImage::ImageViewPlaneEnum CSliceViewer::GetViewPlane(void) const
{
	return m_viewPlane;
}

/**
*/
float CSliceViewer::GetSliceLocation(void) const
{
	return m_sliceLocation;
}

/**
*/
boost::shared_ptr<CTexture> CSliceViewer::GetSliceTexture(void) const
{
	return m_imageDataTexture;
}

/**
*/
void CSliceViewer::SetName(const string &seriesInstanceUID, const string &viewPlaneName, float sliceLocation)
{
#ifdef _MSC_VER
	sprintf_s(m_sliceName, 512, "%s_%s_%.3f)", seriesInstanceUID.c_str(), viewPlaneName.c_str(), sliceLocation);
#else // #ifdef _MSC_VER
	sprintf(m_sliceName, "%s_%s_%.3f)", seriesInstanceUID.c_str(), viewPlaneName.c_str(), sliceLocation);
#endif // #ifdef _MSC_VER
}

/**
*/
bool CSliceViewer::ReadAxialViewPlaneDataFromSeries(boost::shared_ptr<CSeries> series, float sliceLocation)
{
	float depth,
		firstSliceLocation,
		lastSliceLocation,
		*pixel;
	const CSeries::VolumeFacadeType *seriesVolumeFacade;
	int imageCount,
		lastSliceIndex,
		sliceBelowLocationIndex,
		sliceAboveLocationIndex,
		i,
		j;
	const IMedicalImage *image;

	if (!series.get())
	{
		MarkError();

		return true;
	}

	imageCount = series->GetImageCount();

	if (imageCount == 0)
	{
		MarkError();

		return false;
	}
	else if (imageCount == 1)
	{
		depth = 1.0f;

		image = series->GetImage(0);

		if (!image)
		{
			MarkError();

			return false;
		}

		m_sliceLocation = image->GetSliceLocation();
	}
	else
	{
		lastSliceIndex = imageCount - 1;

		firstSliceLocation = series->GetImage(0)->GetSliceLocation();
		lastSliceLocation = series->GetImage(lastSliceIndex)->GetSliceLocation();

		if (firstSliceLocation < lastSliceLocation)
		{
			if (sliceLocation <= firstSliceLocation)
			{
				depth = 1.0f;

				m_sliceLocation = firstSliceLocation;
			}
			else if (sliceLocation >= lastSliceLocation)
			{
				depth = float(lastSliceIndex + 1);

				m_sliceLocation = lastSliceLocation;
			}
			else
			{
				sliceBelowLocationIndex = lastSliceIndex;

				while (sliceLocation <= series->GetImage(sliceBelowLocationIndex)->GetSliceLocation())
					sliceBelowLocationIndex--;

				sliceAboveLocationIndex = 0;

				while (sliceLocation > series->GetImage(sliceAboveLocationIndex)->GetSliceLocation())
					sliceAboveLocationIndex++;

				depth = float(sliceBelowLocationIndex + 1) + (sliceLocation - series->GetImage(sliceBelowLocationIndex)->GetSliceLocation())/(series->GetImage(sliceAboveLocationIndex)->GetSliceLocation() - series->GetImage(sliceBelowLocationIndex)->GetSliceLocation());

				m_sliceLocation = sliceLocation;
			}
		}
		else
		{
			if (sliceLocation >= firstSliceLocation)
			{
				depth = 1.0f;

				m_sliceLocation = firstSliceLocation;
			}
			else if (sliceLocation < lastSliceLocation) //<=
			{
				depth = float(lastSliceIndex + 1);

				m_sliceLocation = lastSliceLocation;
			}
			else
			{
				sliceBelowLocationIndex = lastSliceIndex;

				while (sliceLocation >= series->GetImage(sliceBelowLocationIndex)->GetSliceLocation())
					sliceBelowLocationIndex--;

				sliceAboveLocationIndex = 0;

				while (sliceLocation < series->GetImage(sliceAboveLocationIndex)->GetSliceLocation())
					sliceAboveLocationIndex++;

				depth = float(sliceBelowLocationIndex + 1) + (1.0f - (sliceLocation - series->GetImage(sliceAboveLocationIndex)->GetSliceLocation())/(series->GetImage(sliceBelowLocationIndex)->GetSliceLocation() - series->GetImage(sliceAboveLocationIndex)->GetSliceLocation()));

				m_sliceLocation = sliceLocation;
			}
		}
	}

	seriesVolumeFacade = series->GetVolumeFacade();

	if (seriesVolumeFacade == 0)
	{
		MarkError();

		return false;
	}

	m_width = seriesVolumeFacade->GetWidth();
	m_height = seriesVolumeFacade->GetHeight();

	m_imageData.assign(m_width*m_height, 0.0f);

	pixel = &m_imageData[0];

	for (i=0; i<m_height; ++i)
		for (j=0; j<m_width; ++j)
			*pixel++ = seriesVolumeFacade->GetValue(float(j), float(i), depth);

	m_viewPlane = IMedicalImage::AXIAL_VIEW_PLANE;

	SetName(series->GetSeriesInstanceUID(), "AXIAL", m_sliceLocation);

	return true;
}

/**
*/
bool CSliceViewer::ReadSagittalViewPlaneDataFromSeries(boost::shared_ptr<CSeries> series, float sliceLocation)
{
	// TODO:

	return false;
}

/**
*/
bool CSliceViewer::ReadCoronalViewPlaneDataFromSeries(boost::shared_ptr<CSeries> series, float sliceLocation)
{
	// TODO:

	return false;
}

/**
*/
void CSliceViewer::Create(void)
{
	IImage::Create();

	ISeriesViewer::Create();

	SetName("(INVALID)", "(INVALID)", 0.0f);

	m_viewPlane = IMedicalImage::AXIAL_VIEW_PLANE;

	m_sliceLocation = 0.0f;

	// These values were borrowed from eFilm workstation, which always shows 
	// 400 and 2000 as default values to window center and width, 
	// respectivelly.
	m_windowCenter = 400;
	m_windowWidth = 2000;

	m_imageDataTexture.reset();
}

