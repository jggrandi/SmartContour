#ifndef RAWFROMDICOM2MEDICALIMAGE_INCLUDED
#define RAWFROMDICOM2MEDICALIMAGE_INCLUDED

#include <string>

#include "IMedicalImage.h"

class CRAWFromDICOM2MedicalImage
	: public IMedicalImage
{
public:
	CRAWFromDICOM2MedicalImage(void);
	virtual ~CRAWFromDICOM2MedicalImage(void);
	bool LoadFromFile(const std::string& directoryName, const std::string& fileName);
private:
	bool LoadDataFromFile(const std::string& directoryName);
};

#endif // RAWFROMDICOM2MEDICALIMAGE_INCLUDED

