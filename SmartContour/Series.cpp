#include <algorithm>

#include <ErrorManager.h>

#include "Series.h"

using namespace std;

/**
*/
bool CompareByImageTime(const boost::shared_ptr<IMedicalImage>& firstImage, const boost::shared_ptr<IMedicalImage>& secondImage)
{
	if (firstImage->GetTimeHour() != secondImage->GetTimeHour())
		return firstImage->GetTimeHour() < secondImage->GetTimeHour();
	
	if (firstImage->GetTimeMinute() != secondImage->GetTimeMinute())
		return firstImage->GetTimeMinute() < secondImage->GetTimeMinute();
	
	if (firstImage->GetTimeSecond() != secondImage->GetTimeSecond())
		return firstImage->GetTimeSecond() < secondImage->GetTimeSecond();

	if (firstImage->GetTimeMilisecond() != secondImage->GetTimeMilisecond())
		return firstImage->GetTimeMilisecond() < secondImage->GetTimeMilisecond();

	return false;
}

/**
*/
bool CompareByImageNumber(const boost::shared_ptr<IMedicalImage>& firstImage, const boost::shared_ptr<IMedicalImage>& secondImage)
{
	return firstImage->GetNumber() < secondImage->GetNumber();
}

/**
*/
CSeries::CSeries(void)
{
	Create();
}

/**
*/
CSeries::~CSeries(void)
{
}

/**
*/
const std::string& CSeries::GetSeriesInstanceUID(void) const
{
	return m_seriesInstanceUID;
}

/**
*/
bool CSeries::AddImage(boost::shared_ptr<IMedicalImage> medicalImage)
{
	if (!medicalImage.get())
	{
		MarkError();

		return false;
	}

	if (!medicalImage->IsValid())
	{
		MarkError();

		return false;
	}

	// TODO: default series name.
	if (!m_seriesInstanceUID.empty())
	{
		if (medicalImage->GetSeriesInstanceUID() != m_seriesInstanceUID)
		{
			MarkError();

			return false;
		}

		if (medicalImage->GetSeriesNumber() != m_seriesNumber)
		{
			MarkError();

			return false;
		}
	}
	else
	{
		m_seriesInstanceUID = medicalImage->GetSeriesInstanceUID();

		m_seriesNumber = medicalImage->GetSeriesNumber();
	}

	m_medicalImageArray.push_back(medicalImage);

	if (!SortImageArray(SORT_BY_TIME))
	{
		MarkError();

		return false;
	}

	return true;
}

/**
*/
int CSeries::GetImageCount(void) const
{
	return int(m_medicalImageArray.size());
}

/**
*/
const IMedicalImage *CSeries::GetImage(int imageIndex) const
{
	if (imageIndex >= GetImageCount())
	{
		MarkError();

		return 0;
	}

	return m_medicalImageArray[imageIndex].get();
}

/**
*/
const CSeries::VolumeFacadeType *CSeries::GetVolumeFacade(void) 
{
	if (!m_volumeFacade.get())
	{
		if (!GenerateVolumeFacadeFromImageArray())
		{
			MarkError();

			return 0;
		}
	}

	return m_volumeFacade.get();
}

/**
*/
const CBoundingBox *CSeries::GetVolumeFacadeBoundingBox(void) const
{
	return &m_volumeFacadeBoundingBox;
}

/**
*/
bool CSeries::SortImageArray(ImageSortEnum imageSort)
{
	switch (imageSort)
	{
	case SORT_BY_NUMBER:
		sort(m_medicalImageArray.begin(), m_medicalImageArray.end(), CompareByImageNumber);
		break;
	case SORT_BY_TIME:
		sort(m_medicalImageArray.begin(), m_medicalImageArray.end(), CompareByImageTime);
		break;
	default:
		MarkError();

		return false;
	}

	return true;
}

/**
*/
bool CSeries::GenerateVolumeFacadeFromImageArray(void)
{
	int firstImageWidth,
		firstImageHeight,
		depth,
		i;
	const float *firstImagePixelSpacing,
		*firstImageFirstOrientationAxis,
		*firstImageSecondOrientationAxis;
	vector<float> medicalImageVolumeData;
	vector<boost::shared_ptr<IMedicalImage> >::iterator image;
	double gageKernelParameterArray[3];

	if (m_medicalImageArray.empty())
	{
		MarkError();

		return false;
	}

	firstImageWidth = m_medicalImageArray.front()->GetWidth();
	firstImageHeight = m_medicalImageArray.front()->GetHeight();

	firstImagePixelSpacing = m_medicalImageArray.front()->GetPixelSpacing();

	firstImageFirstOrientationAxis = m_medicalImageArray.front()->GetFirstImageOrientationAxis();
	firstImageSecondOrientationAxis = m_medicalImageArray.front()->GetSecondImageOrientationAxis();
	
	depth = int(m_medicalImageArray.size());

	// Any series is considered a volume, even if there is only one image 
	// available. Since Gage requires at least 2 images to build a volume, the 
	// first and the last images of the series are inserted twice in the 
	// volume.
	medicalImageVolumeData.reserve(firstImageWidth*firstImageHeight*(depth + 2));

	// Inserts a copy of the first image of the series.
	medicalImageVolumeData.insert(medicalImageVolumeData.end(), m_medicalImageArray.front()->GetValueArray(), m_medicalImageArray.front()->GetValueArray() + firstImageWidth*firstImageHeight);

	for (image=m_medicalImageArray.begin(); image!=m_medicalImageArray.end(); image++)
	{
		if (firstImageWidth != (*image)->GetWidth())
		{
			MarkError();

			return false;
		}

		if (firstImageHeight != (*image)->GetHeight())
		{
			MarkError();

			return false;
		}

		if (firstImagePixelSpacing[0] != (*image)->GetPixelSpacing()[0])
		{
			MarkError();

			return false;
		}

		if (firstImagePixelSpacing[1] != (*image)->GetPixelSpacing()[1])
		{
			MarkError();

			return false;
		}

		for (i=0; i<3; ++i)
		{
			if (firstImageFirstOrientationAxis[i] != (*image)->GetFirstImageOrientationAxis()[i])
			{
				MarkError();

				return false;
			}

			if (firstImageSecondOrientationAxis[i] != (*image)->GetSecondImageOrientationAxis()[i])
			{
				MarkError();

				return false;
			}
		}

		medicalImageVolumeData.insert(medicalImageVolumeData.end(), (*image)->GetValueArray(), (*image)->GetValueArray() + firstImageWidth*firstImageHeight);
	}

	for (i=0; i<3; ++i)
	{
		m_firstImageArrayOrientationAxis[i] = firstImageFirstOrientationAxis[i];
		m_secondImageArrayOrientationAxis[i] = firstImageSecondOrientationAxis[i];
	}

	m_volumeFacadeBoundingBox.m_xMin = 0.0f;
	m_volumeFacadeBoundingBox.m_xMax = float(firstImageWidth)*firstImagePixelSpacing[0];
	m_volumeFacadeBoundingBox.m_yMin = 0.0f;
	m_volumeFacadeBoundingBox.m_yMax = float(firstImageHeight)*firstImagePixelSpacing[1];
	m_volumeFacadeBoundingBox.m_zMin = m_medicalImageArray.front()->GetSliceLocation();
	m_volumeFacadeBoundingBox.m_zMax = m_medicalImageArray.back()->GetSliceLocation();

	// Inserts a copy of the last image of the series.
	medicalImageVolumeData.insert(medicalImageVolumeData.end(), m_medicalImageArray.back()->GetValueArray(), m_medicalImageArray.back()->GetValueArray() + firstImageWidth*firstImageHeight);

	m_volumeFacade.reset(new VolumeFacadeType);

	if (!m_volumeFacade.get())
	{
		MarkError();

		return false;
	}

	if (!m_volumeFacade->OpenFromMemory(&medicalImageVolumeData[0], VolumeFacadeType::FLOAT, firstImageWidth, firstImageHeight, depth + 2))
	{
		MarkError();

		return false;
	}

    gageKernelParameterArray[0] = 1.0; 
    gageKernelParameterArray[1] = 0.0;
    gageKernelParameterArray[2] = 0.5;

	//The tent function: f(-1)=0, f(0)=1, f(1)=0, with linear ramps in 
	// between, and zero elsewhere. Used for linear (and bilinear and 
	// trilinear) interpolation.
	if (!m_volumeFacade->SetValueKernel(nrrdKernelTent, gageKernelParameterArray))
	{
		MarkError();

		return false;
	}

	return true;
}

/**
*/
void CSeries::Create(void)
{
	int i;

	// TODO: default series name.
	m_seriesInstanceUID.clear();

	m_seriesNumber = 0;
	
	m_medicalImageArray.clear();
	
	for (i=0; i<3; ++i)
	{
		m_firstImageArrayOrientationAxis[i] = 0.0f;
		m_secondImageArrayOrientationAxis[i] = 0.0f;
	}

	m_volumeFacadeBoundingBox.m_xMin = 0.0f;
	m_volumeFacadeBoundingBox.m_xMax = 0.0f;
	m_volumeFacadeBoundingBox.m_yMin = 0.0f;
	m_volumeFacadeBoundingBox.m_yMax = 0.0f;
	m_volumeFacadeBoundingBox.m_zMin = 0.0f;
	m_volumeFacadeBoundingBox.m_zMax = 0.0f;

	m_volumeFacade.reset();
}

