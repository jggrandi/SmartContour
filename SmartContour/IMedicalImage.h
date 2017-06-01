#ifndef IMEDICALIMAGE_INCLUDED
#define IMEDICALIMAGE_INCLUDED

#include <string>

#include "IImage.h"
#include "GageAdaptor.h"

class IMedicalImage
	: public IImage
{
public:
	enum ImageViewPlaneEnum {
		AXIAL_VIEW_PLANE,
		SAGITTAL_VIEW_PLANE,
		CORONAL_VIEW_PLANE
	};
	IMedicalImage(void);
	virtual ~IMedicalImage(void);
	//virtual bool LoadFromFile(const std::string& directoryName, const std::string& fileName) = 0;
	virtual bool LoadFromFile(boost::shared_ptr<CGageAdaptor> NRRDimage, int imgNumber) =0;
	//virtual bool LoadFromFile(Nrrd* NRRDimage, int imgNumber) =0;
	int GetSeriesNumber(void) const;
	const std::string& GetSeriesInstanceUID(void) const;
	int GetTimeHour(void) const;
	int GetTimeMinute(void) const;
	int GetTimeSecond(void) const;
	int GetTimeMilisecond(void) const;
	int GetNumber(void) const;
	const float *GetPixelSpacing(void) const;
	float GetSliceLocation(void) const;
	const float *GetImagePosition(void) const;
	const float *GetFirstImageOrientationAxis(void) const;
	const float *GetSecondImageOrientationAxis(void) const;
protected:
	virtual void Create(void);
public:
	// The date the acquisition of data that resulted in this image started 
	// (0008x0022).
	int m_acquisitionDateYear;
	int m_acquisitionDateMonth;
	int m_acquisitionDateDay;
	// The date the acquisition of data that resulted in this image started 
	// (0008x0032).
	int m_acquisitionTimeHour;
	int m_acquisitionTimeMinute;
	int m_acquisitionTimeSecond;
	int m_acquisitionTimeMilisecond;
	// Patient's Name (0010,0010).
	std::string m_patientName;
	// Patient's Sex (0010,0040).
	std::string m_patientSex;
	// Patient's Age (0010,1010).
	int m_patientAge;
	// Nominal slice thickness, in mm. (0018,0050).
	float m_sliceThickness;
	// Unique identifier of the Series (0020,000E).
	std::string m_seriesInstanceUID;
	// A number that identifies this series (0020,0011).
	int m_seriesNumber;
	// Instance (form...Image) Number (0020,0013).
	int m_imageNumber;
	// The Image Position (0020,0032) specifies the x, y, and z coordinates of 
	// the upper left hand corner of the image; it is the center of the first 
	// voxel transmitted.
	float m_imagePosition[3];
	// Image Orientation (0020,0037) specifies the direction cosines of the 
	// first row and the first column with respect to the patient. These 
	// Attributes shall be provide as a pair. Row value for the x, y, and z 
	// axes respectively followed by the Column value for the x, y, and z axes 
	// respectively. The direction of the axes is defined fully by the 
	// patient’s orientation. The x-axis is increasing to the left hand side 
	// of the patient. The y-axis is increasing to the posterior side of the 
	// patient. The z-axis is increasing toward the head of the patient. The 
	// patient based coordinate system is a right handed system, i.e. the vector 
	// cross product of a unit vector along the positive x-axis and a unit vector 
	// along the positive y-axis is equal to a unit vector along the positive 
	// z-axis. Note: If a patient lies parallel to the ground, face-up on the 
	// table, with his feet-to-head direction same as the front-to-back 
	// direction of the imaging equipment, the direction of the axes of this 
	// patient based coordinate system and the equipment based coordinate 
	// system in previous versions of this Standard will coincide
	float m_imageOrientation[2][3];
	// The Slice Location (0020,1041) is defined as the relative position of 
	// exposure expressed in mm. This information is relative to an 
	// unspecified implementation specific reference point.
	float m_sliceLocation;
	// Physical distance in the patient between the center of each pixel, 
	// specified by a numeric pair - adjacent row spacing (delimiter) adjacent 
	// column spacing in mm. Required if Volumetric Properties (0008,9206) is 
	// other than DISTORTED or SAMPLED. May be present otherwise (0028,0030).
	float m_pixelSpacing[2];
	// Number of bits allocated for each pixel sample. Each sample shall have 
	// the same number of bits allocated (0028,0100).
	int m_bitsAllocated;
	// Number of bits stored for each pixel sample. Each sample shall have the 
	// same number of bits stored. (0028,0101).
	int m_bitsStored;
	// Most significant bit for pixel sample data. Each sample shall have the 
	// same high bit (0028,0102).
	int m_highBit;
	// Window Center (0028,1050) and Window Width (0028,1051) specify a linear 
	// conversion from the output of the (conceptual) Modality LUT values to 
	// the input to the (conceptual) Presentation LUT. Window Center contains 
	// the value that is the center of the window. Window Width contains the 
	// width of the window.
	int m_windowCenter;
	int m_windowWidth;
};

#endif // IMEDICALIMAGE_INCLUDED

