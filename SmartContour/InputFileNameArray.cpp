//#include <dirent.h>

#include <ErrorManager.h>

#include "InputFileNameArray.h"

using namespace std;

/**
*/
CInputFileNameArray::CInputFileNameArray(void)
{
	Create();
}

/**
*/
CInputFileNameArray::~CInputFileNameArray(void)
{
}

/**
*/
bool CInputFileNameArray::LoadFromFolder(const std::string& directoryName)
{
	string directoryNameFormatted;
	DIR *directoryDescriptor;
	dirent *fileDescriptor;
	InputFileTypeEnum fileType;
	CInputFileInfo dummyFileInfo;

	if (directoryName.empty())
	{
		MarkError();

		return false;
	}

	directoryNameFormatted = directoryName;
	// Pushes a backslash (or a slash) at the end of the directory name.
	if (directoryName.rfind("/") != string::npos)
	{
		if (directoryName.substr(directoryName.size() - 1) != "/")
			directoryNameFormatted = directoryName + "/";
	}
	else
		if (directoryName.substr(directoryName.size() - 1) != "\\")
			directoryNameFormatted = directoryName + "\\";

	directoryDescriptor = opendir(directoryNameFormatted.c_str());

	if (!directoryDescriptor)
	{
		MarkError();

		return false;
	}

	fileDescriptor = readdir(directoryDescriptor);

	while (fileDescriptor)
	{
		fileType = GetFileType(fileDescriptor->d_name);
		//se for uma pasta ignora
		if (fileDescriptor->data.dwFileAttributes != 16)
		{
			switch (fileType) 
			{
			case DICOM_HEADER_FROM_DICOM2:
			case NO_EXTENSION:
				dummyFileInfo.m_fileName = fileDescriptor->d_name;
				dummyFileInfo.m_fileFolderName = directoryNameFormatted;
				dummyFileInfo.m_fileType = DICOM_HEADER_FROM_DICOM2;

				m_fileNameArray.push_back(dummyFileInfo);
				break;
			case NRRD_HEADER:
				dummyFileInfo.m_fileName = fileDescriptor->d_name;
				dummyFileInfo.m_fileFolderName = directoryNameFormatted;
				dummyFileInfo.m_fileType = fileType;

				m_fileNameArray.push_back(dummyFileInfo);
				break;
			case UNKNOWN:
				break;
			default:
				break;
			
			}
		}
		
		fileDescriptor = readdir(directoryDescriptor);
	}

	closedir(directoryDescriptor);

	return true;
}

/**
*/
int CInputFileNameArray::GetInputFileNameCount(void) const
{
	return int(m_fileNameArray.size());
}

/**
*/
const CInputFileNameArray::CInputFileInfo *CInputFileNameArray::GetInputFileInfo(int inputFileIndex) const
{
	if (inputFileIndex < int(m_fileNameArray.size()))
		return &m_fileNameArray[inputFileIndex];

	return 0;
}

/**
*/
CInputFileNameArray::InputFileTypeEnum CInputFileNameArray::GetFileType(const std::string& fileName) const
{
	string fileNameExtension;

	if (fileName.size() < 3)
		return UNKNOWN;

	fileNameExtension = fileName.substr(fileName.size() - 4);
	if (fileNameExtension == "nhdr")
		return NRRD_HEADER;

	fileNameExtension = fileName.substr(fileName.size() - 3);
	if (fileNameExtension == "dcm")
		return DICOM_HEADER_FROM_DICOM2;

	//check if last char is number
	if (isdigit(fileName[fileNameExtension.length()-1]))
		return DICOM_HEADER_FROM_DICOM2;

	if (fileNameExtension == "raw")
		return UNKNOWN;

	return UNKNOWN;
	//return NO_EXTENSION;
	
}

/**
*/
void CInputFileNameArray::Create(void)
{
	m_fileNameArray.clear();
}

