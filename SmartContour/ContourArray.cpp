#include <ErrorManager.h>

#include "ContourArray.h"

using namespace std;

/**
*/
CContourArray::CContourArray(void)
{
}

/**
*/
CContourArray::~CContourArray(void)
{
}

/**
*/
bool CContourArray::AddContour(boost::shared_ptr<CMyContour> contour)
{
	if (!contour.get())
	{
		MarkError();

		return false;
	}

	if (!m_contourArray.empty())
	{
		if (!HasContourAtLocation(contour->GetSliceLocation()))
		{
			if (m_viewPlane != contour->GetSliceViewPlane())
			{
				MarkError();

				return false;
			}
			
			m_contourArray.push_back(contour);
		}
		else
		{
			MarkError();

			return false;
		}
	}
	else
	{
		m_viewPlane = contour->GetSliceViewPlane();
		
		m_contourArray.push_back(contour);
	}

	return true;
}

/**
*/
bool CContourArray::GetContour(IMedicalImage::ImageViewPlaneEnum m_viewPlane, float sliceLocation, boost::shared_ptr<CMyContour>& contour)
{
	vector<boost::shared_ptr<CMyContour> >::iterator contourFromArray;

	for (contourFromArray=m_contourArray.begin(); contourFromArray!=m_contourArray.end(); ++contourFromArray)
		if ((fabsf((*contourFromArray)->GetSliceLocation() - sliceLocation) < 0.5f) && ((*contourFromArray)->GetSliceViewPlane() == m_viewPlane))
		{
			contour = (*contourFromArray);

			return true;
		}

	return false;
}

/**
*/
bool CContourArray::HasContourAtLocation(float sliceLocation) const
{
	vector<boost::shared_ptr<CMyContour> >::const_iterator contour;

	for (contour=m_contourArray.begin(); contour!=m_contourArray.end(); ++contour)
		if (fabsf((*contour)->GetSliceLocation() - sliceLocation) < 0.5f)
			return true;

	return false;
}

/**
*/
bool CContourArray::GetContourArray(std::vector<boost::shared_ptr<CMyContour> >& cArray){
	cArray = m_contourArray;
	return true;
}

/**
*/
void CContourArray::Create(void)
{
	m_viewPlane = IMedicalImage::AXIAL_VIEW_PLANE;

	m_contourArray.clear();
}

bool CContourArray::SaveContourPoints(string fileName)
{
	ofstream contourPointsFile;
	fileName += ".dots";
	contourPointsFile.open(fileName.c_str());

	if(!contourPointsFile.fail() )
	{
		char virgula = ',';
		char endline = '\n';
		char opensub = '(';
		char closesub = ')';
		for (int j = 0 ; j < m_contourArray.size() ; j ++)
		{
			boost::shared_ptr<CMyContour> currentContour = m_contourArray[j];

		//	if (currentContour.get())
			if (currentContour->IsStarted())
			{
				int *pontosControle = currentContour->GetControlPointCoordArray();
				for (int i=0; i<currentContour->GetControlPointCount(); i++)
				{
					int *pontosParaCurve = currentContour->GetParaCurvePointCoordArray(i);
					int posicao = pontosControle[i*2];
					char posicaochar[25];
					itoa (posicao, posicaochar,10);
					contourPointsFile.write (posicaochar,strlen(posicaochar));
				
					contourPointsFile.write(&virgula,1);

					posicao = pontosControle[i*2+1];
					itoa (posicao, posicaochar,10);
					contourPointsFile.write (posicaochar,strlen(posicaochar));
			
					contourPointsFile.write(&virgula,1);
				
					contourPointsFile.write(&opensub,1);
					for (int k=0; k<currentContour->GetParaCurvePointCoordCount(i); k++){
						posicao = pontosParaCurve[k];
						itoa (posicao, posicaochar,10);
						contourPointsFile.write (posicaochar,strlen(posicaochar));

						contourPointsFile.write(&virgula,1);
					}
					contourPointsFile.write(&closesub,1);
				}
			}
			if (j+1<m_contourArray.size())
				contourPointsFile.write( &endline, 1);	
		}
		contourPointsFile.close();
	}
	return true;
}

bool CContourArray::LoadContourPoints(string fileName)
{
	ifstream contourPointsFile;
	contourPointsFile.open(fileName.c_str());
	if(contourPointsFile.fail())
	{
		return false;
	}

	char virgula = ',';
	char endline = '\n';
	char opensub = '(';
	char closesub = ')';
	string line;
	int posXY[6]= {0,0,0,0,0,0};
	int j=0;


    while (! contourPointsFile.eof() )
    {
		boost::shared_ptr<CMyContour> currentContour = m_contourArray[j];
		getline (contourPointsFile,line);
		if (currentContour.get())
		{
			int XouY=0;
			for (int i = 0 ; i < line.size() ; i++)
			{
				if (line[i]==virgula) 
				{	
					if (XouY==5)
					{
						currentContour->LoadContour(posXY);
						XouY = 0;
					}
					else
					{
						XouY++;
					}
				}
				else if (line[i]==opensub)
				{
				}
				else if (line[i]==closesub)
				{
				}
				else
				{
					posXY[XouY] =  atoi(&line[i]);
					while (line[i+1]!=virgula)
						i++;	
				}
			}
		}
		j++;
    }
    contourPointsFile.close();


	return true;
}