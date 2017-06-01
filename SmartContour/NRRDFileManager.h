#ifndef NRRDFILEMANAGER_INCLUDED
#define NRRDFILEMANAGER_INCLUDED

#include <string>

#include "IMedicalImage.h"

class NRRDFileManager
	: public IMedicalImage
{
public:
	NRRDFileManager(void);
	virtual ~NRRDFileManager(void);
	bool LoadFromFile(boost::shared_ptr<CGageAdaptor> NRRDimage, int imgNumber);
private:
	bool LoadDataFromFile(const std::string& directoryName);
};

#endif // NRRDFILEMANAGER_INCLUDED

