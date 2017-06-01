#ifndef RAWIMAGE_INCLUDED
#define RAWIMAGE_INCLUDED

#include <string>

#include "IImage.h"

class CRAWImage
	: public IImage
{
public:
	enum VALUETYPE {
		UNSIGNED_BYTE,
		SHORT,
		UNSIGNED_SHORT
	};
	CRAWImage(void);
	virtual ~CRAWImage(void);
	bool LoadFromFile(int width, int height, VALUETYPE valueType, const std::string& fileName);

};

#endif // RAWIMAGE_INCLUDED

