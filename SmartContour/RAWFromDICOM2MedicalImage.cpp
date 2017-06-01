#include <iostream>
#include <fstream>

#include <boost/tokenizer.hpp>

#include <ErrorManager.h>

#include "RAWFromDICOM2MedicalImage.h"

using namespace std;

/**
Value Representation: DA (Date)
Format: YYYYMMDD
Data length (Byte): 8
*/
template <typename dataType>
bool ReadDateFromDICOM2FileHeader(boost::tokenizer<boost::char_separator<char> >::iterator &token, const boost::tokenizer<boost::char_separator<char> > &tokenArray, dataType *dateYear, dataType *dateMonth, dataType *dateDay)
{
	boost::tokenizer<boost::char_separator<char> >::iterator tokenPart;

	// Skip the field name.
	++token;

	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	// Test the number of elements.
	if (atoi((*token).c_str()) != 1)
	{
		MarkError();

		return false;
	}

	// Skip the number of elements.
	++token;

	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	// Test the value representation.
	if (*token != "DA")
	{
		MarkError();

		return false;
	}

	// Skip the value representation.
	++token;

	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	// Separators of the token.
	boost::char_separator<char> tokenSeparatorArray(".");

	// Tokens of year, month and day.
	boost::tokenizer<boost::char_separator<char> > tokenPartArray(*token, tokenSeparatorArray);

	tokenPart = tokenPartArray.begin();
	
	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	*dateYear = dataType(atoi(tokenPart->c_str()));

	tokenPart++;

	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	*dateMonth = dataType(atoi(tokenPart->c_str()));

	tokenPart++;

	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	*dateDay = dataType(atoi(tokenPart->c_str()));

	return true;
}

/**
Value Representation: DS (Decimal String)
Format: +xxx.xxxx, -xxx.xxxxx, etc
Data length (Byte): 16 (max.)
*/
template <typename dataType, int elementCount>
bool ReadDecimalStringFromDICOM2FileHeader(boost::tokenizer<boost::char_separator<char> >::iterator &token, const boost::tokenizer<boost::char_separator<char> > &tokenArray, dataType *elementDataArray)
{
	int i;

	// Skip the field name.
	++token;

	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	// Test the number of elements.
	if (atoi((*token).c_str()) != elementCount)
	{
		MarkError();

		return false;
	}

	// Skip the number of elements.
	++token;

	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	// Test the value representation.
	if (*token != "DS")
	{
		MarkError();

		return false;
	}

	// Skip the value representation.
	++token;

	if (token == tokenArray.end())
	{
		MarkError();

		return false;
	}

	// Read the element data.
	for (i=0; i<elementCount; ++i)
	{	
		*elementDataArray = dataType(atof(token->c_str()));

		// Skip the ith element.
		++token;

		if (token == tokenArray.end())
		{
			MarkError();

			return false;
		}

		elementDataArray++;
	}

	return true;
}

/**
*/
CRAWFromDICOM2MedicalImage::CRAWFromDICOM2MedicalImage(void)
{
}

/**
*/
CRAWFromDICOM2MedicalImage::~CRAWFromDICOM2MedicalImage(void)
{
}

/**
*/
bool CRAWFromDICOM2MedicalImage::LoadFromFile(const std::string& directoryName, const std::string& fileName)
{
	ifstream fileStream;
	istreambuf_iterator<char> fileStreamBegin,
		fileStreamEnd;
	string fileHeader;
	// Separators of the DICOM2 file header.
    boost::char_separator<char> fileHeaderSeparatorArray(" (),[]\\\t\n");
	boost::tokenizer<boost::char_separator<char> >::iterator token,
		// Many tokens (like UID's) are composed from minor tokens.
		tokenPart;

	if (fileName.empty())
	{
		MarkError();

		return false;
	}

	fileStream.open((directoryName + fileName).c_str());

	if (!fileStream.is_open())
	{
		MarkError();

		return false;
	}

	fileStreamBegin = fileStream;

	fileHeader.assign(fileStreamBegin, fileStreamEnd);

	boost::tokenizer<boost::char_separator<char> > tokenArray(fileHeader, fileHeaderSeparatorArray);

	for (token=tokenArray.begin(); token!=tokenArray.end(); ++token)
	{
		if (*token == "0008")
		{
			++token;

			if (token == tokenArray.end())
			{
				MarkError();

				return false;
			}

			// The date the acquisition of data that resulted in this image 
			// started (0008x0022).
			if (*token == "0022")
			{
				if (!ReadDateFromDICOM2FileHeader<int>(token, tokenArray, &m_acquisitionDateYear, &m_acquisitionDateMonth, &m_acquisitionDateDay))
				{
					MarkError();

					return false;
				}
			}
			// The date the acquisition of data that resulted in this image 
			// started (0008x0032).
			else if (*token == "0033")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "TM")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				// Separators of the token.
				boost::char_separator<char> tokenSeparatorArray(":.");

				boost::tokenizer<boost::char_separator<char> > tokenPartArray(*token, tokenSeparatorArray);

				tokenPart = tokenPartArray.begin();
				
				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_acquisitionTimeHour = atoi(tokenPart->c_str());

				tokenPart++;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_acquisitionTimeMinute = atoi(tokenPart->c_str());

				tokenPart++;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_acquisitionTimeSecond = atoi(tokenPart->c_str());

				tokenPart++;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_acquisitionTimeMilisecond = atoi(tokenPart->c_str());
			}
		}
		else if (*token == "0010")
		{
			++token;

			if (token == tokenArray.end())
			{
				MarkError();

				return false;
			}

			// Patient's Name (0010,0010).
			if (*token == "0010")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "PN")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_patientName = *token;
			}
			// Patient's Sex (0010,0040).
			else if (*token == "0040")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "CS")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_patientSex = *token;
			}
			// Patient's Age (0010,1010).
			else if (*token == "1010")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "AS")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_patientAge = atoi(token->c_str());
			}
		}
		else if (*token == "0018")
		{
			++token;

			if (token == tokenArray.end())
			{
				MarkError();

				return false;
			}

			// Nominal slice thickness, in mm. (0018,0050).
			if (*token == "0050")
			{
				if (!ReadDecimalStringFromDICOM2FileHeader<float, 1>(token, tokenArray, &m_sliceThickness))
				{
					MarkError();

					return false;
				}
			}
		}
		else if (*token == "0020")
		{
			++token;

			if (token == tokenArray.end())
			{
				MarkError();

				return false;
			}

			// Unique identifier of the Series (0020,000E).
			if (*token == "001E")
			{
				// Jumps over the element number.
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				// Tests the element size.
				if (*token != "1")
				{
					MarkError();

					return false;
				}

				// Jumps the element size.
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				// Tests the element representation.
				if (*token != "UI")
				{
					MarkError();

					return false;
				}

				// Jumps the element representation.
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_seriesInstanceUID = *token;
			}
			// A number that identifies this series (0020,0011).
			else if (*token == "0011")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "IS")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_seriesNumber = atoi(token->c_str());
			}
			// Instance (form...Image) Number (0020,0013).
			else if (*token == "0013")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "IS")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_imageNumber = atoi(token->c_str());
			}
			// The Image Position (0020,0032). 
			else if (*token == "0032")
			{
				if (!ReadDecimalStringFromDICOM2FileHeader<float, 3>(token, tokenArray, m_imagePosition))
				{
					MarkError();

					return false;
				}
			}
			// Image Orientation (0020,0037). 
			else if (*token == "0037")
			{
				if (!ReadDecimalStringFromDICOM2FileHeader<float, 6>(token, tokenArray, m_imageOrientation[0]))
				{
					MarkError();

					return false;
				}
			}
			// The Slice Location (0020,1041). 
			else if (*token == "1041")
			{
				if (!ReadDecimalStringFromDICOM2FileHeader<float, 1>(token, tokenArray, &m_sliceLocation))
				{
					MarkError();

					return false;
				}
			}
		}
		else if (*token == "0028")
		{
			++token;

			if (token == tokenArray.end())
			{
				MarkError();

				return false;
			}

			// Number of rows in the image (0028,0010).
			if (*token == "0010")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "US")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_height = atoi(token->c_str());
			}
			// Number of columns in the image (0028,0011).
			else if (*token == "0011")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "US")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_width = atoi(token->c_str());
			}
			// Physical distance in the patient between the center of each 
			// pixel, specified by a numeric pair - adjacent row spacing 
			// (delimiter) adjacent column spacing in mm. Required if 
			// Volumetric Properties (0008,9206) is other than DISTORTED or 
			// SAMPLED. May be present otherwise (0028,0030).
			else if (*token == "0030")
			{
				if (!ReadDecimalStringFromDICOM2FileHeader<float, 2>(token, tokenArray, m_pixelSpacing))
				{
					MarkError();

					return false;
				}
			}
			// Number of bits allocated for each pixel sample. Each sample 
			// shall have the same number of bits allocated (0028,0100).
			else if (*token == "0100")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "US")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_bitsAllocated = atoi(token->c_str());
			}
			// Number of bits stored for each pixel sample. Each sample shall 
			// have the same number of bits stored. (0028,0101).
			else if (*token == "0101")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "US")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_bitsStored = atoi(token->c_str());
			}
			// Most significant bit for pixel sample data. Each sample shall 
			// have the same high bit (0028,0102).
			else if (*token == "0102")
			{
				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "1")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				if (*token != "US")
				{
					MarkError();

					return false;
				}

				++token;

				if (token == tokenArray.end())
				{
					MarkError();

					return false;
				}

				m_highBit = atoi(token->c_str());
			}
			// Window Center (0028,1050) and Window Width (0028,1051) specify 
			// a linear conversion from the output of the (conceptual) 
			// Modality LUT values to the input to the (conceptual) 
			// Presentation LUT. Window Center contains the value that is the 
			// center of the window. Window Width contains the width of the 
			// window.
			else if (*token == "1050")
			{
				if (!ReadDecimalStringFromDICOM2FileHeader<int, 1>(token, tokenArray, &m_windowCenter))
				{
					MarkError();

					return false;
				}
			}
			else if (*token == "1051")
			{
				if (!ReadDecimalStringFromDICOM2FileHeader<int, 1>(token, tokenArray, &m_windowWidth))
				{
					MarkError();

					return false;
				}
			}
		}
	}

	if (!LoadDataFromFile(directoryName))
	{
		MarkError();

		return false;
	}

	return true;
}

/**
*/
bool CRAWFromDICOM2MedicalImage::LoadDataFromFile(const std::string& directoryName)
{
	/*char fileName[1024];
	ifstream fileStream;
	istreambuf_iterator<char> fileStreamBegin,
		fileStreamEnd;
	int fileSize;
	short *fileDataBegin,
		*fileDataEnd;
	vector<char> fileData;

	if (directoryName.empty())
	{
		MarkError();

		return false;
	}

	if (m_bitsAllocated != 16)
	{
		MarkError();

		return false;
	}

	if (m_bitsStored != 16)
	{
		MarkError();

		return false;
	}

	if (m_highBit != 15)
	{
		MarkError();

		return false;
	}

	if (m_width <= 0)
	{
		MarkError();

		return false;
	}

	if (m_height <= 0)
	{
		MarkError();

		return false;
	}

#ifdef _MSC_VER
	sprintf_s(fileName, 1024, "%si%i.raw", directoryName.c_str(), m_imageNumber);
#else // !_MSC_VER
	sprintf(fileName, "%si%i.raw", directoryName.c_str(), m_imageNumber);
#endif // _MSC_VER

	fileStream.open(fileName, ios::binary);

	if (!fileStream.is_open())
	{
		MarkError();

		return false;
	}

	fileStream.seekg(0, ios::end);

	fileSize = int(fileStream.tellg());

	fileStream.seekg(ios::beg);

	fileStreamBegin = fileStream;

	fileData.reserve(fileSize);

	fileData.assign(fileStreamBegin, fileStreamEnd);

	fileSize /= sizeof(short);

	fileDataBegin = (short *)&(*fileData.begin());
	fileDataEnd = fileDataBegin + fileSize;

	m_imageData.reserve(fileSize);

	m_imageData.assign(fileDataBegin, fileDataEnd);*/

	char fileName[1024];
	FILE *fileStream;
	int fileSize;
	short *fileDataBegin,
		*fileDataEnd;
	vector<char> fileData;

	if (directoryName.empty())
	{
		MarkError();

		return false;
	}

	if (m_bitsAllocated != 16)
	{
		MarkError();

		return false;
	}

	if (m_bitsStored != 16)
	{
		MarkError();

		return false;
	}

	if (m_highBit != 15)
	{
		MarkError();

		return false;
	}

	if (m_width <= 0)
	{
		MarkError();

		return false;
	}

	if (m_height <= 0)
	{
		MarkError();

		return false;
	}

#ifdef _MSC_VER
	sprintf_s(fileName, 1024, "%si%i.raw", directoryName.c_str(), m_imageNumber);
#else // !_MSC_VER
	sprintf(fileName, "%si%i.raw", directoryName.c_str(), m_imageNumber);
#endif // _MSC_VER

#ifdef _MSC_VER
	if (fopen_s(&fileStream, fileName, "rb"))
	{
		MarkError();

		return false;
	}
#else // !_MSC_VER
	fileStream = fopen(fileName, "rb");

	if (!fileStream)
	{
		MarkError();

		return false;
	}
#endif // _MSC_VER

	fseek(fileStream, 0L, SEEK_END);

	fileSize = int(ftell(fileStream));

	fseek(fileStream, 0L, SEEK_SET);

	fileData.assign(fileSize, 0);

	fread(&fileData[0], sizeof(char), fileSize, fileStream);

	fileSize /= sizeof(short);

	fileDataBegin = (short *)&(*fileData.begin());
	fileDataEnd = fileDataBegin + fileSize;

	m_imageData.reserve(fileSize);

	m_imageData.assign(fileDataBegin, fileDataEnd);

	return true;
}

