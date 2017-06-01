#ifndef IMAGE_INCLUDED
#define IMAGE_INCLUDED

#include <string>
#include <vector>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

class IImage
	: boost::noncopyable
{
public:
	IImage(void);
	virtual ~IImage(void);
	/**
	*/
	int GetWidth(void) const
	{
		return m_width;
	}
	/**
	*/
	int GetHeight(void) const
	{
		return m_height;
	}
	/**
	*/
	float GetValue(int index) const
	{
		return m_imageData[index];
	}
	/**
	*/
	const float *GetValueArray(void) const
	{
		if (m_imageData.empty())
			return 0;

		return &m_imageData[0];
	}
	/**
	*/
	bool IsValid(void) const
	{
		return (!m_imageData.empty());
	}
protected:
	virtual void Create(void);
public:
	int m_width;
	int m_height;
	std::vector<float> m_imageData;
};

#endif // IMAGE_INCLUDED

