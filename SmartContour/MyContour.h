#ifndef SMARTCONTOUR_INCLUDED
#define SMARTCONTOUR_INCLUDED

#include <vector>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <MyMath.h>

#include "MyLiveWire.h"
#include "SliceViewer.h"

class CMyContour
{
public:
	CMyContour(void);
	virtual ~CMyContour(void);
	bool AttachToSlice(boost::shared_ptr<CSliceViewer> slice);
	float GetSliceLocation(void) const;
	IMedicalImage::ImageViewPlaneEnum GetSliceViewPlane(void) const;
	bool IsStarted(void) const;
	bool IsClosed(void) const;
	bool CMyContour::LoadContour(int posControlPara[]);//---
	bool AddControlPoint(int x, int y);
	bool SetCandidateControlPoint(int x, int y);
	bool RemoveControlPoint(int index);
	bool RemoveLastControlPoint(void);
	bool Close(void);
	bool SelectControlPoint(int x, int y);
	bool MoveControlPoint(int x, int y);
	void Clear(void);
	int GetControlPointCount(void);
	int *GetControlPointCoordArray(void);
	bool ExpandControlPoints(void);
	bool ExpandParaCurvePoints(void);
	int GetContourPointCoordCount(int contourIndex) const;
	const int *GetContourPointCoordArray(int contourIndex) const;
	int GetParaCurvePointCoordCount(int paraCurveIndex) const;//---
	 int *GetParaCurvePointCoordArray(int paraCurveIndex);//---
	bool HaveCandidatePoints(void) const;
	int GetCandidatePointCount(void);
	const int *GetCandidatePointCoordArray(void) const;
	bool SetLiveWireLimits(int xmin , int xmax, int ymin, int ymax);

private:
	void Create(void);
protected:
	int m_sliceWidth;
	int m_sliceHeight;
	float m_sliceLocation;
	IMedicalImage::ImageViewPlaneEnum m_sliceViewPlane;
	boost::shared_ptr<CMyLiveWire> m_liveWire;
	std::vector<int> m_liveWireLimits; 
	bool m_isStarted;
	std::vector<int> m_controlPointCoordArray;
	std::vector<std::vector<int> > m_contourCoordArray;
	std::vector<std::vector<int> > m_paraCurveCoordArray;//---
	bool m_isClosed;
	std::vector<int> m_candidateContourCoordArray;
};

#endif // SMARTCONTOUR_INCLUDED

