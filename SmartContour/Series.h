#ifndef SERIES_INCLUDED
#define SERIES_INCLUDED

#include <vector>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <GageAdaptor.h>
#include <BoundingBox.h>

#include "IMedicalImage.h"

class CSeries
	: boost::noncopyable
{
public:
	// VolumeFacadeType should declare methods "int GetWidth(void)", 
	// "int GetHeight(void)" and "float GetValue(float, float, float)".
	typedef CGageAdaptor VolumeFacadeType;
	enum ImageSortEnum {
		SORT_BY_NUMBER,
		SORT_BY_TIME
	};
	CSeries(void);
	virtual ~CSeries(void);
	const std::string& GetSeriesInstanceUID(void) const;
	bool AddImage(boost::shared_ptr<IMedicalImage> medicalImage);
	int GetImageCount(void) const;
	const IMedicalImage *GetImage(int imageIndex) const;
	const VolumeFacadeType *GetVolumeFacade(void);
	const CBoundingBox *GetVolumeFacadeBoundingBox(void) const;
protected:
	bool SortImageArray(ImageSortEnum imageSort);
	bool GenerateVolumeFacadeFromImageArray(void);
	void Create(void);
protected:
	// Unique identifier of the Series (0020,000E).
	std::string m_seriesInstanceUID;
	// A number that identifies this series (0020,0011).
	int m_seriesNumber;
	std::vector<boost::shared_ptr<IMedicalImage> > m_medicalImageArray;
	float m_firstImageArrayOrientationAxis[3];
	float m_secondImageArrayOrientationAxis[3];
	// Bounding-box (aligned to the image array) in milimeters.
	CBoundingBox m_volumeFacadeBoundingBox;
	boost::shared_ptr<VolumeFacadeType> m_volumeFacade;
};

#endif // SERIES_INCLUDED

