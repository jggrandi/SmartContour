#ifndef ISERIESVIEWER_INCLUDED
#define ISERIESVIEWER_INCLUDED

#include <boost/utility.hpp>

class ISeriesViewer
	: boost::noncopyable
{
public:
	ISeriesViewer(void);
	virtual ~ISeriesViewer(void);
	virtual bool UpdateWindow(int windowCenter, int windowWidth) = 0;
	int GetWindowCenter(void) const;
	int GetWindowWidth(void) const;
protected:
	virtual void Create(void);
protected:
	int m_windowCenter;
	int m_windowWidth;
};

#endif // ISERIESVIEWER_INCLUDED

