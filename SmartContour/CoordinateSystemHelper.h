#ifndef COORDINATESYSTEMHELPER_INCLUDED
#define COORDINATESYSTEMHELPER_INCLUDED

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <PinholeCamera.h>

class CCoordinateSystemHelper
	: boost::noncopyable
{
public:
	CCoordinateSystemHelper(void);
	virtual ~CCoordinateSystemHelper(void);
	bool AttachToCamera(boost::shared_ptr<CPinholeCamera> camera);
	bool ScreenToImage(int x, int y, float *s, float *t);
//	bool ImageToScreen(float x, float y, int *s, int *t);
private:
	void Create(void);
protected:
	boost::shared_ptr<CPinholeCamera> m_attachedCamera;
};

#endif // COORDINATESYSTEMHELPER_INCLUDED

