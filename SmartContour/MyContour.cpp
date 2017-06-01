#include <ErrorManager.h>

#include "MyContour.h"

using namespace std;

/**
*/
CMyContour::CMyContour(void)
{
	Create();
}

/**
*/
CMyContour::~CMyContour(void)
{
}

bool CMyContour::SetLiveWireLimits(int xmin , int xmax, int ymin, int ymax){
	if (xmin > xmax){
		m_liveWireLimits.push_back(xmax);
		m_liveWireLimits.push_back(xmin);
	} else {
		m_liveWireLimits.push_back(xmin);
		m_liveWireLimits.push_back(xmax);
	}
	if (ymin>ymax){
		m_liveWireLimits.push_back(ymax);
		m_liveWireLimits.push_back(ymin);
	} else{
		m_liveWireLimits.push_back(ymin);
		m_liveWireLimits.push_back(ymax);
	}

	return true;
}

/**
*/
bool CMyContour::AttachToSlice(boost::shared_ptr<CSliceViewer> slice)
{
	Create();

	if (!slice.get())
	{
		MarkError();

		return false;
	}

	if (!slice->IsValid())
	{
		MarkError();

		return false;
	}

	if (m_liveWireLimits.empty())
	{
		if (!m_liveWire->LoadFromImage(slice))
		{
			MarkError();

			return false;
		}
	}
	else{
		if (!m_liveWire->LoadFromImage(slice,m_liveWireLimits[0],m_liveWireLimits[1],m_liveWireLimits[2],m_liveWireLimits[3]))
		{
			MarkError();

			return false;
		}
	}

	m_sliceWidth = slice->GetWidth();
	m_sliceHeight = slice->GetHeight();

	m_sliceLocation = slice->GetSliceLocation();

	m_sliceViewPlane = slice->GetViewPlane();

	return true;
}

/**
*/
float CMyContour::GetSliceLocation(void) const
{
	return m_sliceLocation;
}

/**
*/
IMedicalImage::ImageViewPlaneEnum CMyContour::GetSliceViewPlane(void) const
{
	return m_sliceViewPlane;
}

/**
*/
bool CMyContour::IsStarted(void) const
{
	return m_isStarted;
}

/**
*/
bool CMyContour::IsClosed(void) const
{
	return m_isClosed;
}

/**
*/
bool CMyContour::LoadContour(int posControlPara[])
{
	if (m_isClosed)
	{
		MarkError();

		return false;
	}

	m_controlPointCoordArray.push_back(posControlPara[0]);
	m_controlPointCoordArray.push_back(posControlPara[1]);

	vector<int> _candidateParaCurveControl;//---

	_candidateParaCurveControl.push_back(posControlPara[2]);//---
	_candidateParaCurveControl.push_back(posControlPara[3]);//---
	_candidateParaCurveControl.push_back(posControlPara[4]);//---
	_candidateParaCurveControl.push_back(posControlPara[5]);//---
	
	m_paraCurveCoordArray.push_back(_candidateParaCurveControl);//---


	if (!m_liveWire->AddSeed(posControlPara[0], posControlPara[1]))
	{
		MarkError();
		
		return false;
	}

	m_isStarted = true;
	return true;
}
/**
*/
bool CMyContour::AddControlPoint(int x, int y)
{
	if (m_isClosed)
	{
		MarkError();

		return false;
	}

	m_controlPointCoordArray.push_back(x);
	m_controlPointCoordArray.push_back(y);

	m_contourCoordArray.push_back(m_candidateContourCoordArray);
	
	vector<int> _candidateParaCurveControl;//---
	int aux;//---
	if(m_candidateContourCoordArray.size()>4){
		aux= m_candidateContourCoordArray.size()/4;//---
		if (aux % 2 != 0){//---
			aux++;//---
		}//---
		_candidateParaCurveControl.push_back(m_candidateContourCoordArray[aux*3]);//---
		_candidateParaCurveControl.push_back(m_candidateContourCoordArray[aux*3+1]);//---
		_candidateParaCurveControl.push_back(m_candidateContourCoordArray[aux]);//---
		_candidateParaCurveControl.push_back(m_candidateContourCoordArray[aux+1]);//---
	}else{
		
		_candidateParaCurveControl.push_back(x);//---
		_candidateParaCurveControl.push_back(y);//---
		_candidateParaCurveControl.push_back(x);//---
		_candidateParaCurveControl.push_back(y);//---
	}
	m_paraCurveCoordArray.push_back(_candidateParaCurveControl);//---

	
	m_candidateContourCoordArray.clear();

	if (!m_liveWire->AddSeed(x, y))
	{
		MarkError();
		
		return false;
	}

	m_isStarted = true;

	return true;
}

/**
*///---
bool CMyContour::ExpandControlPoints(void) 
{
	if (!m_controlPointCoordArray.empty())
	{

		for (int i=0; i<m_controlPointCoordArray.size()/2-1; i++){
			if (!m_liveWire->AddSeed(m_paraCurveCoordArray[i][2], m_paraCurveCoordArray[i][3]))
			{
				MarkError();
				
				return false;
			}
			if (!SetCandidateControlPoint(m_paraCurveCoordArray[i+1][0],m_paraCurveCoordArray[i+1][1]))
			{
				MarkError();
					
				return false;
			}
			if(m_candidateContourCoordArray.size()>6){
				int aux= m_candidateContourCoordArray.size()/2;//---
				if (aux % 2 != 0){//---
					aux++;//---
				}//---
				m_controlPointCoordArray[i*2] = m_candidateContourCoordArray[aux];
				m_controlPointCoordArray[i*2+1] = m_candidateContourCoordArray[aux+1];
			}	
		}

		if (!m_liveWire->AddSeed(m_controlPointCoordArray[m_controlPointCoordArray.size() - 2], m_controlPointCoordArray[m_controlPointCoordArray.size() - 1]))
		{
			MarkError();
			
			return false;
		}
	}
	else
		m_isStarted = false;
	return true;
}
bool CMyContour::ExpandParaCurvePoints(void) 
{
	if (!m_controlPointCoordArray.empty())
	{

		for (int i=0; i<m_controlPointCoordArray.size()/2-1; i++){
			if (!m_liveWire->AddSeed(m_controlPointCoordArray[i*2], m_controlPointCoordArray[i*2+1]))
			{
				MarkError();
				
				return false;
			}
			if (!SetCandidateControlPoint(m_controlPointCoordArray[i*2+2], m_controlPointCoordArray[i*2+3]))
			{
				MarkError();
					
				return false;
			}
			if(m_candidateContourCoordArray.size()>4){
				int aux= m_candidateContourCoordArray.size()/3;//---
				if (aux % 2 != 0){//---
					aux++;//---
				}//---
				m_paraCurveCoordArray[i+1][2] = m_candidateContourCoordArray[aux];
				m_paraCurveCoordArray[i+1][3] = m_candidateContourCoordArray[aux+1];
				m_paraCurveCoordArray[i+1][0] = m_candidateContourCoordArray[aux*2];
				m_paraCurveCoordArray[i+1][1] = m_candidateContourCoordArray[aux*2+1];
			}
			m_candidateContourCoordArray.clear();
		}

		if (!m_liveWire->AddSeed(m_controlPointCoordArray[m_controlPointCoordArray.size() - 2], m_controlPointCoordArray[m_controlPointCoordArray.size() - 1]))
		{
			MarkError();
			
			return false;
		}
	}
	else
		m_isStarted = false;

	return true;
}

/**
*/
bool CMyContour::SetCandidateControlPoint(int x, int y)
{
	boost::shared_ptr<std::vector<int> > candidateContourIndexArray;
	std::vector<int>::iterator candidateContourIndex;

/*	if (m_isClosed)
	{
		MarkError();

		return false;
	}
*/
	candidateContourIndexArray = m_liveWire->GetShortestPathFromSeed(x, y);

	m_candidateContourCoordArray.clear();

	for (candidateContourIndex=candidateContourIndexArray->begin(); candidateContourIndex!=candidateContourIndexArray->end(); ++candidateContourIndex)
	{
		m_candidateContourCoordArray.push_back((*candidateContourIndex)%m_sliceWidth);
		m_candidateContourCoordArray.push_back((*candidateContourIndex)/m_sliceWidth);
	}

	return true;
}

/**
*/
bool CMyContour::RemoveControlPoint(int index)
{
	return false;
}

/**
*/
bool CMyContour::RemoveLastControlPoint(void)
{
	if (m_isClosed)
	{
		m_isClosed=false;
		//MarkError();

		//return false;
		return true;
	}

/*	if (m_controlPointCoordArray.empty())
	{
		MarkError();

		return false;
	}
*/
	m_controlPointCoordArray.pop_back();
	m_controlPointCoordArray.pop_back();

	m_contourCoordArray.pop_back();
	m_paraCurveCoordArray.pop_back();//---

	m_candidateContourCoordArray.clear();

	if (!m_controlPointCoordArray.empty())
	{
		if (!m_liveWire->AddSeed(m_controlPointCoordArray[m_controlPointCoordArray.size() - 2], m_controlPointCoordArray[m_controlPointCoordArray.size() - 1]))
		{
			MarkError();
			
			return false;
		}
	}
	else
		m_isStarted = false;

	return true;
}

/**
*/
bool CMyContour::Close(void)
{
	/*if (m_isClosed)
	{
		MarkError();

		return false;
	}

	if (m_controlPointCoordArray.empty())
	{
		MarkError();

		return false;
	}*/

/*	if (!SetCandidateControlPoint(m_controlPointCoordArray[0], m_controlPointCoordArray[1]))
	{
		MarkError();

		return false;
	}

	if (!AddControlPoint(m_controlPointCoordArray[0], m_controlPointCoordArray[1]))
	{
		MarkError();

		return false;
	}
*/
	m_candidateContourCoordArray.clear();
	m_isClosed = true;

	return true;
}

/**
*/
bool CMyContour::SelectControlPoint(int x, int y)
{
	return false;
}

/**
*/
bool CMyContour::MoveControlPoint(int x, int y)
{
	return false;
}

/**
*/
void CMyContour::Clear(void)
{
	m_isStarted = false;

	m_controlPointCoordArray.clear();

	m_contourCoordArray.clear();

	m_paraCurveCoordArray.clear();//---

	m_isClosed = false;
}

/**
*/
int CMyContour::GetControlPointCount(void)
{
	return int(m_controlPointCoordArray.size()/2);
}

/**
*/
int *CMyContour::GetControlPointCoordArray(void) 
{
	if (m_controlPointCoordArray.empty())
		return 0;

	return &m_controlPointCoordArray[0];
}

/**
*/
int CMyContour::GetContourPointCoordCount(int contourIndex) const
{
	if (contourIndex >= int(m_contourCoordArray.size()))
		return 0;

	return int(m_contourCoordArray[contourIndex].size());
}

/**
*/
const int *CMyContour::GetContourPointCoordArray(int contourIndex) const
{
	if (contourIndex >= int(m_contourCoordArray.size()))
		return 0;

	if (m_contourCoordArray[contourIndex].empty())
		return 0;

	return &m_contourCoordArray[contourIndex][0];
}
/**
*///---
int CMyContour::GetParaCurvePointCoordCount(int paraCurveIndex) const
{
	if (paraCurveIndex >= int(m_paraCurveCoordArray.size()))
		return 0;

	return int(m_paraCurveCoordArray[paraCurveIndex].size());
}

/**
*///---
int *CMyContour::GetParaCurvePointCoordArray(int paraCurveIndex)
{
	if (paraCurveIndex >= int(m_paraCurveCoordArray.size()))
		return 0;

	if (m_paraCurveCoordArray[paraCurveIndex].empty())
		return 0;

	return &m_paraCurveCoordArray[paraCurveIndex][0];
}
/**
*/
bool CMyContour::HaveCandidatePoints(void) const
{
	return !m_candidateContourCoordArray.empty();
}

/**
*/
int CMyContour::GetCandidatePointCount(void)
{
	return int(m_candidateContourCoordArray.size()/2);
}

/**
*/
const int *CMyContour::GetCandidatePointCoordArray(void) const
{
	if (m_candidateContourCoordArray.empty())
		return 0;

	return &m_candidateContourCoordArray[0];
}

/**
*/
void CMyContour::Create(void)
{
	m_sliceWidth = 0;
	m_sliceHeight = 0;

	m_sliceLocation = 0.0f;

	m_sliceViewPlane = IMedicalImage::AXIAL_VIEW_PLANE;

	m_liveWire.reset(new CMyLiveWire);
	
	m_isStarted = false;

	m_controlPointCoordArray.clear();

	m_contourCoordArray.clear();

	m_paraCurveCoordArray.clear();//---

	m_isClosed = false;

	m_candidateContourCoordArray.clear();
}

