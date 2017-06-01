#ifndef CONTOURARRAY_INCLUDED
#define CONTOURARRAY_INCLUDED

#include <string>
#include <vector>
#include <fstream>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "IMedicalImage.h"
#include "MyContour.h"

class CContourArray
	: boost::noncopyable
{
public:
	CContourArray(void);
	virtual ~CContourArray(void);
	bool AddContour(boost::shared_ptr<CMyContour> contour);
	bool GetContour(IMedicalImage::ImageViewPlaneEnum m_viewPlane, float sliceLocation, boost::shared_ptr<CMyContour>& contour);
	bool GetContourArray(std::vector<boost::shared_ptr<CMyContour> >& cArray);

	bool SaveContourPoints(std::string fileName);
	bool LoadContourPoints(std::string fileName);
	bool SaveContourImages();

protected:
	bool HasContourAtLocation(float sliceLocation) const;
	void Create(void);
protected:
	IMedicalImage::ImageViewPlaneEnum m_viewPlane;
	std::vector<boost::shared_ptr<CMyContour> > m_contourArray;
};

#endif // CONTOURARRAY_INCLUDED

