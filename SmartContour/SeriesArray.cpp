#include <ErrorManager.h>

//#include <teem/nrrd.h>
//#include <teem/gage.h>

//#include <boost/utility.hpp>
//#include <boost/shared_ptr.hpp>

#include "SeriesArray.h"
#include "RAWFromDICOM2MedicalImage.h"
#include "NRRDFileManager.h"
#include "GageAdaptor.h"


using namespace std;

/**
*/
CSeriesArray::CSeriesArray(void)
{
	Create();
}

/**
*/
CSeriesArray::~CSeriesArray(void)
{
}

/**
 * abre as imagens a partir do header nrrd e coloca num array de imagens
*/
bool CSeriesArray::LoadFromFileNameArray(boost::shared_ptr<CInputFileNameArray> inputFileNameArray)
{
	int inputFileInfoCount,
		i;
	const CInputFileNameArray::CInputFileInfo *inputFileInfo;
	boost::shared_ptr<IMedicalImage> dummyMedicalImage;

	if (!inputFileNameArray.get())
	{
		MarkError();
		return false;
	}

	inputFileInfoCount = inputFileNameArray->GetInputFileNameCount();

	if (inputFileInfoCount == 0)
	{
		MarkError();
		return false;
	}

	for (i=0; i<inputFileInfoCount; ++i)
	{
		inputFileInfo = inputFileNameArray->GetInputFileInfo(i);

		if (inputFileInfo == 0)
		{
			MarkError();
			return false;
		}

		switch (inputFileInfo->m_fileType)
		{
		case CInputFileNameArray::NRRD_HEADER:
		{
			boost::shared_ptr<CGageAdaptor> m_image;
			double kernelParam[3];

			// Scale parameter, in units of samples.
			kernelParam[0] = 1.0; 
			// B.
			kernelParam[1] = 0.0;
			// C.
			kernelParam[2] = 0.5;

			m_image.reset(new CGageAdaptor);

			if (!m_image.get())
			{
				MarkError();
				return false;
			}

	//		const std::string& path = inputFileInfo->m_fileFolderName+inputFileInfo->m_fileName;
			if (!m_image->Open(inputFileInfo->m_fileName.c_str()))
			{
				MarkError();
				return false;
			}

			
			for (int i= 0 ; i <  m_image->GetDepth(); i++){
				dummyMedicalImage.reset(new NRRDFileManager);
				if (!dummyMedicalImage.get())
				{
					MarkError();					
					return false;
				}

				if (!dummyMedicalImage->LoadFromFile(m_image, i))
				{
					MarkError();					
					return false;
				}
				if (!AddMedicalImageToSeriesArray(dummyMedicalImage))
				{
					MarkError();
					return false;
				}
			}
		}
			break;
		/*case CInputFileNameArray::DICOM_HEADER_FROM_DICOM2:
			//case DICOM file, convert to raw

 			dummyMedicalImage.reset(new CRAWFromDICOM2MedicalImage);

			if (!dummyMedicalImage.get())
			{
				MarkError();				
				return false;
			}

			if (!dummyMedicalImage->LoadFromFile(inputFileInfo->m_fileFolderName, inputFileInfo->m_fileName))
			{
				MarkError();				
				return false;
			}

			if (!AddMedicalImageToSeriesArray(dummyMedicalImage))
			{
				MarkError();
				return false;
			}
			break;
*/
		case CInputFileNameArray::UNKNOWN:
			MarkError();

			return false;
		}
	}

	return true;
}

/**
*/
int CSeriesArray::GetSeriesCount(void) const
{
	return int(m_seriesArray.size());
}

/**
*/
boost::shared_ptr<CSeries> CSeriesArray::GetSeries(int seriesIndex)
{
	return m_seriesArray[seriesIndex];
}

/**
*/
bool CSeriesArray::AddMedicalImageToSeriesArray(boost::shared_ptr<IMedicalImage> medicalImage)
{
	vector<boost::shared_ptr<CSeries> >::iterator series;
	boost::shared_ptr<CSeries> dummySeries;

	for (series=m_seriesArray.begin(); series!=m_seriesArray.end(); ++series)
		if ((*series)->GetSeriesInstanceUID() == medicalImage->GetSeriesInstanceUID())
			break;

	if (series != m_seriesArray.end())
	{
		if (!(*series)->AddImage(medicalImage))
		{
			MarkError();

			return false;
		}
	}
	
	else
	{
		dummySeries.reset(new CSeries);

		if (!dummySeries.get())
		{
			MarkError();

			return false;
		}

		if (!dummySeries->AddImage(medicalImage))
		{
			MarkError();

			return false;
		}

		m_seriesArray.push_back(dummySeries);
	}

	return true;
}

/**
*/
void CSeriesArray::Create(void)
{
	m_seriesArray.clear();
}

