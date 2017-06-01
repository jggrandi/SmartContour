#include <fstream>
#include <algorithm>

#include <ErrorManager.h>

#include "RAWImage.h"

using namespace std;

/**
*/
CRAWImage::CRAWImage(void)
{
}

/**
*/
CRAWImage::~CRAWImage(void)
{
}

/**
*/
bool CRAWImage::LoadFromFile(int width, int height, VALUETYPE valueType, const std::string& fileName)
{
	ifstream fileStream;
	istreambuf_iterator<char> fileStreamBegin,
		fileStreamEnd;
	int fileSize;
	vector<char> fileData;

	if (fileName.empty())
	{
		MarkError();

		return false;
	}

	if (width <= 0)
	{
		MarkError();

		return false;
	}

	if (height <= 0)
	{
		MarkError();

		return false;
	}

	fileStream.open(fileName.c_str(), ios::binary);

	if (!fileStream.is_open())
	{
		MarkError();

		return false;
	}

	fileStream.seekg(0, ios::end);

	fileSize = int(fileStream.tellg());

	fileStream.seekg(ios::beg);

	fileStreamBegin = fileStream;

	fileData.assign(fileStreamBegin, fileStreamEnd);

	switch (valueType)
	{
	case UNSIGNED_BYTE:
		{
			unsigned char *fileDataBegin,
				*fileDataEnd;

			fileDataBegin = (unsigned char *)&(*fileData.begin());
			fileDataEnd = fileDataBegin + fileSize;

			m_imageData.assign(fileDataBegin, fileDataEnd);
		}
		break;
	case SHORT:
		{
			short *fileDataBegin,
				*fileDataEnd;

			fileSize /= sizeof(short);

			fileDataBegin = (short *)&(*fileData.begin());
			fileDataEnd = fileDataBegin + fileSize;

			m_imageData.assign(fileDataBegin, fileDataEnd);
		}
		break;
	case UNSIGNED_SHORT:
		{
			unsigned short *fileDataBegin,
				*fileDataEnd;

			fileSize /= sizeof(unsigned short);

			fileDataBegin = (unsigned short *)&(*fileData.begin());
			fileDataEnd = fileDataBegin + fileSize;

			m_imageData.assign(fileDataBegin, fileDataEnd);
		}
		break;
	default:
		MarkError();

		return false;
	}

	m_width = width;
	m_height = height;

	return true;
}
