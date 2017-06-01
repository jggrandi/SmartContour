#ifndef INPUTFILENAMEARRAY_INCLUDED
#define INPUTFILENAMEARRAY_INCLUDED

#include <string>
#include <vector>

#include <dirent.h>

#include <boost/utility.hpp>

class CInputFileNameArray
	: boost::noncopyable
{
public:
	enum InputFileTypeEnum {
		UNKNOWN,
		DICOM_HEADER_FROM_DICOM2,
		NRRD_HEADER,
		NO_EXTENSION
	};
	class CInputFileInfo
	{
	public:
		/**
		*/
		CInputFileInfo(void)
		{
			m_fileName.clear();
			m_fileFolderName.clear();
			m_fileType = UNKNOWN;
		}
		/**
		*/
		CInputFileInfo(const CInputFileInfo& inputFileInfo)
			: m_fileName(inputFileInfo.m_fileName),
			m_fileFolderName(inputFileInfo.m_fileFolderName),
			m_fileType(inputFileInfo.m_fileType)
		{
		}
		/**
		*/
		CInputFileInfo& operator=(const CInputFileInfo& inputFileInfo)
		{
			m_fileName = inputFileInfo.m_fileName;
			m_fileFolderName = inputFileInfo.m_fileFolderName;
			m_fileType = inputFileInfo.m_fileType;

			return *this;
		}
	public:
		std::string m_fileName;
		std::string m_fileFolderName;
		InputFileTypeEnum m_fileType;
	};
	CInputFileNameArray(void);
	virtual ~CInputFileNameArray(void);
	bool LoadFromFolder(const std::string& directoryName);
	int GetInputFileNameCount(void) const;
	const CInputFileInfo *GetInputFileInfo(int inputFileIndex) const;
private:
	InputFileTypeEnum GetFileType(const std::string& fileName) const;
	void Create(void);
protected:
	std::vector<CInputFileInfo> m_fileNameArray;
};

#endif // INPUTFILENAMEARRAY_INCLUDED

