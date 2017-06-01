#ifndef SLICE_INCLUDED
#define SLICE_INCLUDED

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include <Texture.h>

#include "IMedicalImage.h"
#include "Series.h"
#include "ISeriesViewer.h"

class CSliceViewer
	: public IImage,
	public ISeriesViewer
{
public:
	CSliceViewer(void);
	virtual ~CSliceViewer(void);
	bool Initialize(boost::shared_ptr<CSeries> series, IMedicalImage::ImageViewPlaneEnum viewPlane, float sliceLocation, int windowCenter, int windowWidth);
	const char *GetName(void) const;
	bool UpdateWindow(int windowCenter, int windowWidth);
	IMedicalImage::ImageViewPlaneEnum GetViewPlane(void) const;
	float GetSliceLocation(void) const;
	boost::shared_ptr<CTexture> GetSliceTexture(void) const;
private:
	void SetName(const std::string &seriesInstanceUID, const std::string &viewPlaneName, float sliceLocation);
	bool ReadAxialViewPlaneDataFromSeries(boost::shared_ptr<CSeries> series, float sliceLocation);
	bool ReadSagittalViewPlaneDataFromSeries(boost::shared_ptr<CSeries> series, float sliceLocation);
	bool ReadCoronalViewPlaneDataFromSeries(boost::shared_ptr<CSeries> series, float sliceLocation);
	void Create(void);
protected:
	char                              m_sliceName[512];
	IMedicalImage::ImageViewPlaneEnum m_viewPlane;
	float                             m_sliceLocation;
	boost::shared_ptr<CTexture>       m_imageDataTexture;
};

#endif // SLICE_INCLUDED

