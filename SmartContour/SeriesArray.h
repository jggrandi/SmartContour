#ifndef SERIESARRAY_INCLUDED
#define SERIESARRAY_INCLUDED

#include <vector>
#include <string>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "InputFileNameArray.h"
#include "Series.h"

class CSeriesArray
	: boost::noncopyable
{
public:
	CSeriesArray(void);
	virtual ~CSeriesArray(void);
	bool LoadFromFileNameArray(boost::shared_ptr<CInputFileNameArray> inputFileNameArray);
	int GetSeriesCount(void) const;
	boost::shared_ptr<CSeries> GetSeries(int seriesIndex);

	std::vector<boost::shared_ptr<CSeries> > m_seriesArray; //it was protected
private:
	bool AddMedicalImageToSeriesArray(boost::shared_ptr<IMedicalImage> medicalImage);
	void Create(void);
protected:
	
};

#endif // SERIESARRAY_INCLUDED

