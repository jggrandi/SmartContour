#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

/**
 * Adapter of OpenGL extensions.
 */
#include <GL/glew.h>

#include <string.h>
#include <queue>

//dicom
#include "imebra/imebra/include/imebra.h"

#include <Commdlg.h>
#include <iostream>
#include <fstream>

#include <AntTweakBar.h>
//#include <QApplication>
//#include <QProgressBar>

/**
 * Adapter of OpenGL Windows-specific extensions.
 */
#ifdef _WIN32
#include <GL/wglew.h>
#else // !_WIN32
#include <GL/glxew.h>
#endif // _WIN32

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <string>

/** Smart-pointer class. Takes care of shared pointers in the application. **/
#include <boost/shared_ptr.hpp>

#include <ErrorManager.h>
#include <PinholeCamera.h>
#include <Texture.h>
#include <Timer.h>
#include <MyMath.h>

#include "InputFileNameArray.h"
#include "SeriesArray.h"
#include "Series.h"
#include "SliceViewer.h"
#include "MyLiveWire.h"
#include "CoordinateSystemHelper.h"
#include "MyContour.h"
#include "ContourArray.h"

using namespace std;


/** These are the live variables ***/
int   main_window;
bool  show_text = 0;

#define SLICE_FORWARD_ID			10
#define PREPROCESS_ID				15
#define SLICE_BACKWARD_ID			20
#define CLEAN_CONTOUR				30
#define SAVE_CONTOUR				40
#define SAVE_CONTOUR_POINTS			45
#define LOAD_CONTOUR_POINTS			46
#define CONTOUR_ADD_REMOVE_ID		50
#define CONTOUR_EDIT_ID				60
#define REGULAGEM_NAV_ID			70
#define REGULAGEM_JANELA_ID			80
#define BOUNDINGBOX_ID				90
#define CLOSE_CONTOUR_ID			100
#define COPY_CONTOUR_ID				200
#define PASTE_CONTOUR_ID			300
#define EXPAND_CONTROL_POINTS		400
#define EXPAND_PARA_CURVE_POINTS	500
#define CMD_CLOSE_ID				102
#define SLICE_ID					104
#define OPEN_DICOM_FILE				106
#define OPEN_DICOM_FOLDER			108
#define OPEN_RAW					110

#define INI_WINDOW_WIDTH			1024
#define INI_WINDOW_HEIGHT			800
#define INIT_WINDOW_CENTER			128
#define INIT_WINDOW_WIDTH			256

#define ENTER_KEY					13

float estado[3];
int selecionado[3];
float boundingBox[4] = {0,0,0,0};
bool WorkspaceIsDefined = false;
bool definingWorkspace = false;
int m_height = 0;
int m_width = 0;
int m_depth = 0;
bool firstTime = true;

//ant tweak bar variables
TwBar *bar;			// Pointer to a tweak bar
float barColor[] = { 1.0f, 0.0f, 0.0f};
unsigned int slice = 1, last_slice = 1;
bool filter = false;
int window_w, window_l;
bool show_workspace = false;

//QT variables
//QProgressBar *progressBar;

string m_saveName;
string m_loadName;

//variables required to nhdr files
string nhdrFileName, pacient, columns, rows, bitsAllocated, spacing;
int framesCount = 0;

/******************************************************************************
 * Interface with application classes. 
 *****************************************************************************/

/**
 * Container of image series (an image series is a set of coplanar images).
 */
boost::shared_ptr<CSeriesArray>   m_seriesArray;
/**
 * Series slice currently being updated/displayed (if enabled). All operations 
 * on contours are (currently) based on this slice.
 */
boost::shared_ptr<CSliceViewer>   m_currentSlice;

/**
 * Container of contours. Each contour is associated with one slice.
 */
boost::shared_ptr<CContourArray>  m_contourArray;
/**
 * Contour currently being updated/displayed (if enabled).
 */
boost::shared_ptr<CMyContour>     m_currentContour;
/**
 * Contour pointer currently saved.
 */
boost::shared_ptr<CMyContour>     m_savedContour;

/**
 * Distance between slices used when traveling along the series in a given 
 * image plane.
 */
//float							m_offsetSlices = 92.0f;
float                           m_distanceBetweenSlices = 1.0f;

/******************************************************************************
 * Camera parameters.
 *****************************************************************************/

/**
 * The current operation modes of the application. They're exclusive (by now), 
 * which means that only one can be enabled at any time.
 */
enum ApplicationModeEnum {
	// In this mode, the left button of the mouse displaces the image in the XY 
	// axis of the camera and the right button of the mouse changes camera's 
	// near plane size.
	LBUTTON2PAN_RBUTTON2ZOOM,
	// In this mode, the left button of the mouse adds a new point to the current 
	// contour (m_currentContour) and the right button of the mouse removes the 
	// last point added to it.
	LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR,
	// In this mode, the left button of the mouse changes the window (transfer 
	// function) of the current slice (m_currentSlice).
	LBUTTON2WINDOW,
	// Just a flag to indicate the last valid application mode.
	LAST_MOUSE_MODE,

	EDITCONTOURMODE,
	BOUNDINGBOXMODE,
	BEGIN

};
char m_applicationModeDescriptionArray[BEGIN+1][512] = {
	{"Application mode - left button: pan; right button: zoom."},
	{"Application mode - left button: add point to contour; right button: remove last point from contour."},
	{"Application mode - left button: window center/width; right button: (none)."},
	{"LAST_MOUSE_MODE "},
	{"EDITCONTOURMODE "},
	{"Application mode - BOUNDINGBOX "},
	{"Select an image or a directory to open..."}
};
/**
 * The camera instance.
 */
boost::shared_ptr<CPinholeCamera> m_camera;
/**
 * Stores the current application mode. It defaults to pan and zoom.
 */
ApplicationModeEnum               m_applicationMode = BEGIN;
/**
 * Dummy class designed to translate between different application coordinate 
 * systems (camera, screen, slice, etc).
 */
CCoordinateSystemHelper           m_coordinateSystemHelper;
/**
 * Mouse mode and position containers.
 */
int                               m_currentMouseButton = 0;
int                               m_currentMousePositionX = -1;
int                               m_currentMousePositionY = -1;

/******************************************************************************
 * Application interface.
 *****************************************************************************/

bool SetApplicationMode(ApplicationModeEnum applicationMode);
ApplicationModeEnum GetApplicationMode(void);
bool LoadFromFolder(const std::string& folderName);
bool UpdateCurrentSlice(IMedicalImage::ImageViewPlaneEnum imageViewPlane , float location);
bool UpdateCurrentSliceWindow(int xMouseDisplacement, int yMouseDisplacement);
bool MoveCurrentSliceForward(void);
bool MoveCurrentSlice(void);
bool MoveCurrentSliceBackward(void);
bool ExpandControlPoints(void);
bool ExpandParaCurvePoints(void);
bool PasteCurrentContour(void);
bool CopyCurrentContour(void);
bool LoadCurrentContour(bool createNewContour);
bool AddControlPointToCurrentContour(int x, int y);
bool UpdateNextControlPointOfCurrentContour(int x, int y);
bool RemoveLastControlPointOfCurrentContour(void);
bool CloseCurrentContour(void);
void MoveControlPoints(int _x,int _y);
void OnSize(GLsizei w, GLsizei h);
void setMode( int control );
void SelectControlPoints(int _x, int _y);
void onSaveContourPoints(std::string fileSaveName);
void onLoadContourPoints(std::string fileLoadName);
void floodFill(short *imagem, int x, int y, int newColor,int oldColor);
bool onSave(string saveNamePointer);  
bool LoadRaw();
bool LoadDicomFolder();
bool NhdrLoad(const std::string& folderName);
boost::shared_ptr<CInputFileNameArray> LoadInputDicomArray(const std::string& folderName);
bool Filter(string file, int bytes, int imageSize, int frames, string paciente, string spacing);
bool CreateNhdrFile(string file);

void OnPaint();
bool TweakBarCreate(int slices);

bool InitializeCamera(void);


bool CreateNhdrFile(string file)
{
	string nhdrFileName = file;
	nhdrFileName.append(".nhdr");
	ofstream nhdrFile; 
	nhdrFile.open(nhdrFileName.c_str());
	nhdrFile << "NRRD0001\n";	
	nhdrFile << "content: " << pacient << "\n";
	nhdrFile << "dimension: 3\n";
	nhdrFile << "type: " << bitsAllocated << endl;
	nhdrFile << "sizes: " << rows << " " << columns << " " << framesCount << "\n";
	nhdrFile << "spacings: " << spacing << "\n";
	nhdrFile << "endian: little\n";	
	string rawFile = file.substr(file.rfind("\\")+1);
	rawFile.append(".raw");
	nhdrFile << "data file: ./" << rawFile << "\n";
	//TODO: try with zip, rar, gzip, etc
	nhdrFile << "encoding: raw\n";
	nhdrFile.close();

	return true;
}

/**
 *
 */
bool SetApplicationMode(ApplicationModeEnum applicationMode)
{
	switch (applicationMode)
	{
	case BOUNDINGBOXMODE:

		m_applicationMode = BOUNDINGBOXMODE;		
		break;
	case LBUTTON2PAN_RBUTTON2ZOOM:
		m_applicationMode = LBUTTON2PAN_RBUTTON2ZOOM;
		break;
	case LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR:
		if (WorkspaceIsDefined)
		{
			m_applicationMode = LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR;
			
			if (!LoadCurrentContour(true))
			{
				MarkError();
				return false;
			}			
		}
		else m_applicationMode = BOUNDINGBOXMODE;		
		break;
	case LBUTTON2WINDOW:
		m_applicationMode = LBUTTON2WINDOW;
		break;
	case EDITCONTOURMODE:
		m_applicationMode = EDITCONTOURMODE;
		break;
	case BEGIN:
		m_applicationMode = BEGIN;
		break;
	default:
		MarkError();
		return false;
	}

	//cout << m_applicationModeDescriptionArray[m_applicationMode] << endl;

	return true;
}

/**
 *
 */
ApplicationModeEnum GetApplicationMode(void)
{
	return m_applicationMode;
}


/**
 * Apply filter on the images (histogram equalization)
 */
bool Filter()
{
	//progressBar->setRange(1, m_depth);
	//progressBar->reset();
	//progressBar->show();

	//TODO: diferenciar numero de bytes
	if (bitsAllocated.compare("short")!=0) return false;
	int bytes = 2; //short

	int imageSize = atoi(columns.c_str())*atoi(rows.c_str());
	
	FILE* input, *output;
	short* buffer = (short*)calloc(imageSize, bytes);
	short* FilteredImage = (short*)calloc(imageSize, bytes);

	string file = nhdrFileName;
	file.erase(file.size()-4);
	file.append("raw");
	input = fopen(file.c_str(),"rb");
	file.erase(file.size()-4);
	file.insert(file.rfind("\\")+1, "filtered\\");
	file.append("_filtered.raw");
	CreateDirectory("filtered", NULL);
	output = fopen(file.c_str() ,"wb");

	for (short i=0; i<framesCount; i++)
	{
		//read one frame and apply filter
		fread(buffer,bytes,imageSize,input);
		vector<short> original;
		for (int i=0; i<imageSize; i++)
			original.push_back(buffer[i]);
		
		//calculate histogram
		vector<short> histogram(256, 0);
		for (int i=0; i<imageSize; i++)
			histogram[original[i]]++;				
		
		vector<int> cumhist(256, 0);
		//Important: black pixels outside the patient's body supersaturate the cumulative histogram
		//they aren't in our calculations
		histogram[0]=1;
		cumhist[0]=0;

		//calculate cumulative histogram
		for (short i=1; i<256; i++)
			cumhist[i]= cumhist[i-1] + histogram[i];

		int s=0;
		for (short i=0; i<256; i++)
			s += histogram[i];		

		vector<int> t(256, 0), tx(256, 0);
		for (short i=0; i<256; i++)
			t[i] = floor(((double)256/s)*cumhist[i] + 0.5);

		vector<short> newimage(imageSize,0);
		
		tx[255] = t[255];
		for (short i=1; i<256; i++)
			tx[i-1]=t[i];

		for (int i=0; i<imageSize; i++)
			if (original[i]!=0)
				newimage[i]=tx[original[i]];
			else newimage[i]=t[0];
		
		for (int i=0; i<imageSize; i++)
			FilteredImage[i] = newimage[i];

		//save filtered image
		fwrite(FilteredImage , bytes , imageSize , output );
		//int p = progressBar->value();
		//progressBar->setValue( ++p );
	}
	fclose(input);
	fclose(output);

	free (buffer);
	free (FilteredImage);
	
	string rawFile = file;
	file.erase(file.size()-4);
	CreateNhdrFile(file);

	//progressBar->hide();
	file.erase(file.rfind("\\")+1);
/*
	if (!NhdrLoad(file))
	{
		MarkError();			
		return false;
	}
*/
	return true;
}

/**
 * Receive an array of DICOM files and create a single raw and a single nhdr file
 */
bool Dicom2Raw(boost::shared_ptr<CInputFileNameArray> inputFileNameArray)
{
	int inputFileInfoCount, i;
	const CInputFileNameArray::CInputFileInfo *inputFileInfo;
	boost::shared_ptr<IMedicalImage> dummyMedicalImage;
	string Codec, modality, photometricInterpretation;
	string outputFileName, rawFileName;
	framesCount = 0;

	if (!inputFileNameArray.get())
	{
		MarkError();
		return false;
	}

	inputFileInfoCount = inputFileNameArray->GetInputFileNameCount();

	if (inputFileInfoCount == 0)
	{
		MarkError();
		return false;
	}
	
	//using the first image to build nrrd header
	int index=0;

	inputFileInfo = inputFileNameArray->GetInputFileInfo(index);
	if (inputFileInfo == 0)
	{
		MarkError();
		return false;
	}

	while (inputFileInfo->m_fileType != CInputFileNameArray::DICOM_HEADER_FROM_DICOM2)
	{
		index++;
		inputFileInfo = inputFileNameArray->GetInputFileInfo(index);
		if (inputFileInfo == 0)
		{
			MarkError();
			return false;
		}
	}

	//progressBar initialization
	//progressBar->setRange(1, inputFileInfoCount);
	//progressBar->reset();
	//progressBar->show();
	//int p = progressBar->value();
	//progressBar->setValue(++p);

	string dcmFile = inputFileInfo->m_fileName.c_str();
	string dcmFolder = inputFileInfo->m_fileFolderName.c_str();

	using namespace puntoexe;
	using namespace puntoexe::imebra;

	// Open the file containing the dicom dataset
	ptr<puntoexe::stream> inputStream(new puntoexe::stream);
	inputStream->openFile(dcmFile, std::ios_base::in);
	ptr<puntoexe::streamReader> reader(new streamReader(inputStream));

	// Get a codec factory and let it use the right codec to create a dataset
	//  from the input stream
	ptr<codecs::codecFactory> codecsFactory(codecs::codecFactory::getCodecFactory());
	ptr<dataSet> loadedDataSet(codecsFactory->load(reader, 2048));

	pacient = loadedDataSet->getString(0x0010, 0, 0x0010, 0);
	std::cout << "Pacient: " << pacient << '\n';

	rows = loadedDataSet->getString(0x0028, 0, 0x0010, 0);
	rows.erase(rows.rfind('.'));
	std::cout << "Rows: " << rows << '\n'; 

	columns = loadedDataSet->getString(0x0028, 0, 0x0011, 0);
	columns.erase(columns.rfind('.'));
	std::cout << "Columns: " << columns << '\n';

	spacing = loadedDataSet->getString(0x0028, 0, 0x0030, 0);
	if (spacing.empty()) spacing="1.0";	
	if (spacing.compare("0") == 0) spacing="1.0";
	spacing.erase(spacing.rfind('.')+4);
	spacing.append(" ");
	spacing.append(spacing);
	spacing.append("1");
	std::cout << "Spacing: "<< spacing << '\n';

	Codec = loadedDataSet->getString(0x0002, 0, 0x0010, 0);
	std::cout << "Transfer Syntax: " << Codec << '\n';

	bitsAllocated = loadedDataSet->getString(0x0028, 0, 0x0100, 0);
	bitsAllocated.erase(bitsAllocated.rfind('.'));
	std::cout << "Bits: " << bitsAllocated << '\n';
	
	photometricInterpretation = loadedDataSet->getString(0x0028, 0, 0x0004, 0);
	std::cout << "Photometric Interpretation: " << photometricInterpretation << '\n';

	modality = loadedDataSet->getString(0x0008, 0, 0x0060, 0);
	std::cout << "Modality: " << modality << '\n';

	outputFileName.assign(dcmFolder.c_str());	
	outputFileName.erase(outputFileName.size()-1);
	outputFileName.append(outputFileName.substr(outputFileName.rfind('\\')));		

	rawFileName = outputFileName;
	rawFileName.append(".raw");

	ptr<puntoexe::stream> outputStream(new puntoexe::stream);
	outputStream->openFile(rawFileName, std::ios_base::out | std::ios_base::trunc);	

	std::cout << "Converting to " << rawFileName << endl;
			
	//percorre o array e converte cada imagem para raw, gerando um unico arquivo raw
	for (i=0; i<inputFileInfoCount; ++i)
	{
		inputFileInfo = inputFileNameArray->GetInputFileInfo(i);

		if (inputFileInfo == 0)
		{
			MarkError();
			return false;
		}

		if (inputFileInfo->m_fileType == CInputFileNameArray::DICOM_HEADER_FROM_DICOM2)
		{
			string dcmFile = inputFileInfo->m_fileName.c_str();

			using namespace puntoexe;
			using namespace puntoexe::imebra;

			// Open the file containing the dicom dataset
			ptr<puntoexe::stream> inputStream(new puntoexe::stream);
			inputStream->openFile(dcmFile, std::ios_base::in);
			ptr<puntoexe::streamReader> reader(new streamReader(inputStream));

			// Get a codec factory and let it use the right codec to create a dataset
			//  from the input stream
			ptr<codecs::codecFactory> codecsFactory(codecs::codecFactory::getCodecFactory());
			ptr<dataSet> loadedDataSet(codecsFactory->load(reader, 2048));	

			std::wstring codec(Codec.length(), L' ');
			std::copy(Codec.begin(), Codec.end(), codec.begin());		
										
			try
			{
				// Scan through the frames
				for(imbxUint32 frameNumber(0);; ++frameNumber)
				{			
					// Apply the modality VOI/LUT
					ptr<transforms::modalityVOILUT> modVOILUT(new transforms::modalityVOILUT);
					modVOILUT->declareDataSet(loadedDataSet);
					modVOILUT->declareInputImage(0, loadedDataSet->getImage(frameNumber));
					modVOILUT->doTransform();
					ptr<image> finalImage(modVOILUT->getOutputImage(0));

					// Apply the presentation VOI/LUT
					ptr<transforms::VOILUT> presentationVOILUT(new transforms::VOILUT);
					presentationVOILUT->declareDataSet(loadedDataSet);
					imbxUint32 firstVOILUTID(presentationVOILUT->getVOILUTId(0));
					if(firstVOILUTID != 0)
					{
						presentationVOILUT->setVOILUT(firstVOILUTID);
						presentationVOILUT->declareInputImage(0, finalImage);
						presentationVOILUT->doTransform();
						finalImage = presentationVOILUT->getOutputImage(0);
					}

					// Adjust high bit
					if(finalImage->getHighBit() != 7)
					{
						ptr<image> eightBitImage(new image);
						imbxUint32 width, height;
						finalImage->getSize(&width, &height);
						eightBitImage->create(width, height, image::depthU8, L"YBR_FULL", 7);
						ptr<transforms::transformHighBit> modifyHighBit(new transforms::transformHighBit);
						modifyHighBit->declareDataSet(loadedDataSet);
						modifyHighBit->declareInputImage(0, finalImage);
						modifyHighBit->declareOutputImage(0, eightBitImage);
						modifyHighBit->doTransform();
						finalImage = eightBitImage;
					}
			
					//write raw file
					std::cout << dcmFile << endl;
					ptr<puntoexe::stream> outputStream(new puntoexe::stream);														
					outputStream->openFile(rawFileName, std::ios_base::out | std::ios_base::app);				
					ptr<puntoexe::streamWriter> writer(new streamWriter(outputStream));		
					ptr<codecs::codec> outputCodec(codecsFactory->getCodec(codec));
					outputCodec->setImage(writer, finalImage, codec, codecs::codec::high,
						"OB", atoi(bitsAllocated.c_str()), false, false, false, false);
							
					++framesCount;
					//progressBar->setValue(++p);
				}
			}

			catch(dataSetImageDoesntExist&)
			{
				//This exception is thrown when we reach the end of the images list
				exceptionsManager::getMessage();
			}
		}
	}
	
	//std::cout  << rawFileName << " created\n ";
	//progressBar->hide();

	rawFileName.erase(rawFileName.size()-4);
	if (bitsAllocated == "8") bitsAllocated = "uchar";
	if (bitsAllocated == "16") bitsAllocated = "short";
	CreateNhdrFile(rawFileName);

	rawFileName.append(".nhdr");
	nhdrFileName = rawFileName;

	//Filter(rawFileName,atoi(bitsAllocated.c_str())/8, atoi(rows.c_str())*atoi(columns.c_str()),framesCount);

	return true;
}

/**
 *	Uses windows tool to select file to open
 */
bool OpenFileWindow(char file[255], char type[255])
{
	OPENFILENAME ofn;
	char szfile[255];

	SecureZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL ;
	ofn.lpstrFile = file ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szfile );
	ofn.lpstrFilter = type;
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if (GetOpenFileName( &ofn )) return true;
	else return false;
}

/**
 *	Create a single raw and nrrd files including all dicom files inside a folder 
 */
bool LoadDicomFolder()
{
	//openfile window
	char szFile[255];
	for (int i=0; i<255; i++)
		szFile[i]=NULL;

	char type[255] = "DICOM\0*.DCM\0All\0*.*\0";
	if (!OpenFileWindow(szFile,type)) return true;

	//get folder name
	std::string FolderName(szFile, strlen(szFile));
	size_t barPos(FolderName.rfind('\\'));
	if(barPos != FolderName.npos)
		FolderName.erase(barPos+1);

	//convert all files in folder to raw
	if (!Dicom2Raw(LoadInputDicomArray(FolderName)))
	{
		MarkError();
		return false;
	}	

	//load nhdr file and update screen
	if (!NhdrLoad(FolderName))
	{
		MarkError();
		return false;
	}

	return true;
}

/**
 * Load a raw file using a nhdr header
 */
bool LoadRaw()
{
	SetApplicationMode(BEGIN);
	
	char szFile[255] ;
	for (int i=0; i<255; i++)
		szFile[i]=NULL;

	char type[255] = "NHDR\0*.NHDR\0All\0*.*\0";
	if (!OpenFileWindow(szFile,type)) return true;

	nhdrFileName = szFile;

	//get informations from nhdr
	ifstream nhdrFile;
	nhdrFile.open(nhdrFileName);
	string line;
	getline(nhdrFile, line);
	getline(nhdrFile, line);
	pacient = line.substr(line.find(" ")+1);
	getline(nhdrFile, line);
	getline(nhdrFile, line);
	bitsAllocated = line.substr(6);
	getline(nhdrFile, line);
	int i, j;
	sscanf (line.c_str(),"%*s %d %d %d", &i, &j, &framesCount);
	stringstream r, c;
	r << i;
	rows = r.str();
	c << j;
	columns = c.str();
	getline(nhdrFile, line);
	spacing = line.substr(line.find(" ")+1);
	nhdrFile.close();

	string FolderName(szFile, strlen(szFile));
	size_t dotPos(FolderName.rfind('\\'));
	if(dotPos != FolderName.npos)
		FolderName.erase(dotPos+1);
	 
	//load nhdr file and update window
	if (!NhdrLoad(FolderName))
	{
		MarkError();			
		return false;
	}

	return true;
}

/**
 * Load a nhdr file created by this program
 */
bool NhdrLoad(const std::string& folderName)
{	
	firstTime = true;
	show_text = true;
	WorkspaceIsDefined = false;
	OnPaint();
	glutPostRedisplay();
	show_text = false;

	firstTime = false;
	
	if (!LoadFromFolder(folderName))
	{
		MarkError();
		return false;
	}

	if (!InitializeCamera())
	{
		MarkError();
		return false;
	}

	if (!m_currentSlice->UpdateWindow(INIT_WINDOW_CENTER, INIT_WINDOW_WIDTH))
	{
		MarkError();
		return false;
	}

	m_width  = m_currentSlice->GetWidth();
	m_height = m_currentSlice->GetHeight();	
	m_depth = m_currentSlice->GetSliceLocation();

	window_l = m_currentSlice->GetWindowCenter();
	window_w = m_currentSlice->GetWindowWidth();

	TweakBarCreate(m_depth);

	return true;
}


/**
 * Find all dicom files under 'folderName' folder and put into an array
 */
boost::shared_ptr<CInputFileNameArray> LoadInputDicomArray(const std::string& folderName)
{
	// Stores the name of the files under 'folderName'.
	boost::shared_ptr<CInputFileNameArray> fileNameArray;

	fileNameArray.reset(new CInputFileNameArray);
	if (!fileNameArray.get())
	{
		MarkError();
		//return false;
	}

	// Scans the source folder and stores its file names.
	if (!fileNameArray->LoadFromFolder(folderName))
	{
		MarkError();
		//return false;
	}

	return fileNameArray;
}

/**
 * Loads an image series from the set of images under 'folderName' folder. 
 * It was designed to deal with eFilm folders, where each series is stored in 
 * its own folder.
 */
bool LoadFromFolder(const std::string& folderName)
{
	CTimer                                 timer;
	// Stores the name of the files under 'folderName'.
	boost::shared_ptr<CInputFileNameArray> fileNameArray;

	//cout << "Loading source files under " << folderName << " folder..." << endl;

	fileNameArray.reset(new CInputFileNameArray);

	if (!fileNameArray.get())
	{
		MarkError();
		return false;
	}

	// Scans the source folder and stores its file names.
	if (!fileNameArray->LoadFromFolder(folderName))
	{
		MarkError();
		return false;
	}

	m_seriesArray.reset(new CSeriesArray);

	if (!m_seriesArray.get())
	{
		MarkError();
		return false;
	}

	// Loads the series array from the files in the source folder.	
	if (!m_seriesArray->LoadFromFileNameArray(fileNameArray))
	{
		MarkError();

		return false;
	}

	m_currentSlice.reset(new CSliceViewer);

	if (!m_currentSlice.get())
	{
		MarkError();
		return false;
	}

	// Initializes the current slice.
	if (!UpdateCurrentSlice(IMedicalImage::AXIAL_VIEW_PLANE, MyMath::GetMaximumPositiveValue<float>()))
	{
		MarkError();
		return false;
	}

	//cout << "Initialization elapsed time: " << timer.GetElapsed() << "\n\n";

	return true;
}

/**
 * Loads the current slice (the slice which is displayed an where all 
 * operations are performed)
 */
bool UpdateCurrentSlice(IMedicalImage::ImageViewPlaneEnum imageViewPlane , float location)
{
	// Loads the slice from the current series.
	if (!m_currentSlice->Initialize(m_seriesArray->GetSeries(0)
		                            , IMedicalImage::AXIAL_VIEW_PLANE
									, location
									, m_currentSlice->GetWindowCenter()
									, m_currentSlice->GetWindowWidth()))
	{
		MarkError();

		return false;
	}

	if (!LoadCurrentContour(GetApplicationMode() == LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR))
	{
		MarkError();

		return false;
	}

	//cout << "Slice " << m_currentSlice->GetName() << " loading elapsed time: " << timer.GetElapsed() << endl;

	return true;
}

/**
 * Adjust window's settings of the current slice.
 */
bool UpdateCurrentSliceWindow(int xMouseDisplacement, int yMouseDisplacement)
{
	if (!m_currentSlice->UpdateWindow(m_currentSlice->GetWindowCenter() + 5*xMouseDisplacement, m_currentSlice->GetWindowWidth() - 5*yMouseDisplacement))
	{
		MarkError();

		return false;
	}
	else {
		window_l = m_currentSlice->GetWindowCenter();
		window_w = m_currentSlice->GetWindowWidth();
	}
	
	//cout << "Window center/width: " << m_currentSlice->GetWindowCenter() << "/" << m_currentSlice->GetWindowWidth() << endl;

	return true;
}

/**
 * Troca slice pela próxima
 */
bool MoveCurrentSliceForward(void)
{
	if (!UpdateCurrentSlice(IMedicalImage::AXIAL_VIEW_PLANE, m_currentSlice->GetSliceLocation() + m_distanceBetweenSlices))
	{
		MarkError();

		return false;
	}
	//cout << m_currentSlice->GetWidth() << " " << m_currentSlice->GetHeight() << " " << m_currentSlice->GetSliceLocation() << " " << endl;

	return true;
}

/**
 *Troca slice pela anterior
 */
bool MoveCurrentSliceBackward(void)
{
	if (!UpdateCurrentSlice(IMedicalImage::AXIAL_VIEW_PLANE, m_currentSlice->GetSliceLocation() - m_distanceBetweenSlices))
	{
		MarkError();

		return false;
	}
	//cout << m_currentSlice->GetWidth() << " " << m_currentSlice->GetHeight() << " " << m_currentSlice->GetSliceLocation() << " " << endl;

	return true;
}

/**
*Expande pontos de controle usando Livewires, de acordo com o 
*posicionamento dos pontos das curvas paramétricas
*/
bool ExpandControlPoints(void){
	if (!m_currentContour->ExpandControlPoints())
	{
		MarkError();

		return false;
	}
	return true;
}
bool ExpandParaCurvePoints(void){
	if (!m_currentContour->ExpandParaCurvePoints())
	{
		MarkError();

		return false;
	}

	return true;
}

/**
*Cola contorno copiado para slice atual
*/
bool PasteCurrentContour(void){
	const int * m_controlPointArray = m_savedContour->GetControlPointCoordArray();
	if (m_currentContour==m_savedContour)
		return true;
	m_currentContour->Clear();
	if (!m_savedContour.get())
	{
		MarkError();
		return false;
	}
	for (int i=0; i<m_savedContour->GetControlPointCount(); i++){	
		if (!m_currentContour->SetCandidateControlPoint(m_controlPointArray[i*2],m_controlPointArray[i*2+1]))
		{
			MarkError();			
			return false;
		}

		if (!m_currentContour->AddControlPoint(m_controlPointArray[i*2],m_controlPointArray[i*2+1]))
		{
			MarkError();			
			return false;
		}
	}

/*
	if (!ExpandControlPoints())
	{
		MarkError();

		return false;
	}
	*/
	return true;
}

/**
*copia ponteiro do contorno da slice atual
*/
bool CopyCurrentContour(void){
	CTimer timer;

	//cout << "Copying contour from slice " << m_currentSlice->GetName() << "..." << endl;



	if (!m_contourArray->GetContour(m_currentSlice->GetViewPlane()
		                            , m_currentSlice->GetSliceLocation()
								    , m_savedContour))
		return false;


	//cout << "Countour copied in " << timer.GetElapsed() << " ms." << endl;

	return true;
}

/**
 * Loads the contour associated to the current slice to start (or continue) the 
 * contour updating. If there's no contour associated to the current slice, a 
 * blank contour is created.
 */
bool LoadCurrentContour(bool createNewContour)
{
	CTimer timer;

	//cout << "Loading contour from slice " << m_currentSlice->GetName() << "..." << endl;

	// Initializes the contour array in the first call of this function.
	if (!m_contourArray.get())
	{
		m_contourArray.reset(new CContourArray);

		if (!m_contourArray.get())
		{
			MarkError();

			return false;
		}
	}

	m_currentContour.reset(new CMyContour);

	if (!m_currentContour.get())
	{
		MarkError();

		return false;
	}
	
	m_currentContour->SetLiveWireLimits(boundingBox[0],boundingBox[1],boundingBox[2],boundingBox[3]);
	// Tryies to load the contour associated to the current slice. If it can't, 
	// creates a blank contour and associates it to the current slice
	if (!m_contourArray->GetContour(m_currentSlice->GetViewPlane()
		                            , m_currentSlice->GetSliceLocation()
								    , m_currentContour) && createNewContour)
	{
		
		if (!m_currentContour->AttachToSlice(m_currentSlice))
		{
			MarkError();

			return false;
		}
		if (!m_contourArray->AddContour(m_currentContour))
		{
			MarkError();

			return false;
		}

	}

	//cout << "Countour loaded in " << timer.GetElapsed() << " ms." << endl;

	return true;
}

/**
 * Adds a control point to the current contour.
 */
bool AddControlPointToCurrentContour(int x, int y)
{
	if (!m_currentContour->IsClosed())
	{
		float s
			  , t;

		if (!m_coordinateSystemHelper.ScreenToImage(x, glutGet(GLUT_WINDOW_HEIGHT) - y, &s, &t))
		{
			MarkError();
			return false;
		}

		if (!m_currentContour->AddControlPoint(int(s), int(t)))
		{
			MarkError();			
			return false;
		}
	}

	return true;
}

/**
 * Updates the position of the next control point of the current contour.
 */
bool UpdateNextControlPointOfCurrentContour(int x, int y)
{
	float s
		  , t;

	if (m_currentContour->IsStarted() && !m_currentContour->IsClosed())
	{
		if (!m_coordinateSystemHelper.ScreenToImage(x, glutGet(GLUT_WINDOW_HEIGHT) - y, &s, &t))
		{
			MarkError();

			return false;
		}

		if (!m_currentContour->SetCandidateControlPoint(int(s), (int)t))
		{
			MarkError();
			
			return false;
		}
	}

	return true;
}

/**
 * Removes the last control point of the current contour.
 */
bool RemoveLastControlPointOfCurrentContour(void)
{
	if (!m_currentContour->RemoveLastControlPoint())
	{
		MarkError();
			
		return false;
	}
	return true;
}

/**
 *Close current contour
 */
bool CloseCurrentContour(void)
{
	if (GetApplicationMode() == LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR)
		if (!m_currentContour->Close())
		{
			MarkError();

			return false;
		}

	return true;
}

/**
 * Initializes the camera. The current settings are only to 2D visualization.
 */
bool InitializeCamera(void)
{
	// Parameters of the 3D camera settings (disabled).
	//float deviceWindowWidth,
	//	deviceWindowHeight,
	//	nearPlaneDepth,
	//	farPlaneDepth,
	//	fieldOfView;

	m_camera.reset(new CPinholeCamera);

	if (!m_camera.get())
	{
		MarkError();

		return false;
	}

	// 2D camera settings, calculated from the slice bounding box and screen 
	// settings.

	if (firstTime)
	{
		if (!m_camera->Create(0.0
							  , glutGet(GLUT_WINDOW_WIDTH)
							  , 0.0f
							  , glutGet(GLUT_WINDOW_HEIGHT)//m_currentSlice->GetHeight()
							  , 0.0f
							  , 1.0f
							  , glutGet(GLUT_WINDOW_WIDTH)
							  , glutGet(GLUT_WINDOW_HEIGHT)))
		{
			MarkError();
			return false;
		}
	}
	
	else if (!m_camera->Create(0.0
		                  , m_currentSlice->GetWidth()
						  , 0.0f
						  , m_currentSlice->GetHeight()
						  , 0.0f
						  , 1.0f
						  , glutGet(GLUT_WINDOW_WIDTH)
						  , glutGet(GLUT_WINDOW_HEIGHT)))
		{
			MarkError();
			return false;
		}

	m_camera->ResetViewMatrix();

	// 3D camera settings, derived from the volume (series) bounding box 
	// and screen settings (disabled).
	//deviceWindowWidth = m_volumeBoundingBox->m_xMax - m_volumeBoundingBox->m_xMin;
	//deviceWindowHeight = m_volumeBoundingBox->m_yMax - m_volumeBoundingBox->m_yMin;
	//
	//nearPlaneDepth = min(fabs(m_volumeBoundingBox->m_zMin), fabs(m_volumeBoundingBox->m_zMax));
	//farPlaneDepth = max(fabs(m_volumeBoundingBox->m_zMin), fabs(m_volumeBoundingBox->m_zMax));
	//		
	// The field of view is calcuted to match the slice with the near plane. 
	// It comes from the definition of "gluPerspective", where we see that 
	// "0.5f*deviceWindowHeight = nearPlaneDepth*tan(fieldOfView*M_PI/360.0);".
	//fieldOfView = (360.0f/MyMath::Pi<float>())*atan2f(0.5f*deviceWindowHeight, nearPlaneDepth);
	//
	//if (!m_camera->Create(fieldOfView, nearPlaneDepth, farPlaneDepth, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)))
	//{
	//	MarkError();
	//
	//	return false;
	//}
	//
	//m_camera->MoveFront(-nearPlaneDepth - 0.00001f);
	//m_camera->MoveSide(-0.5f*deviceWindowWidth);
	//m_camera->MoveUp(-0.5f*deviceWindowHeight);

	if (!m_coordinateSystemHelper.AttachToCamera(m_camera))
	{
		MarkError();

		return false;
	}

	return true;
}

/**
 *
 */
void OnDestroy(void)
{
	int windowName;

	windowName = glutGetWindow();

	if (windowName)
		glutDestroyWindow(windowName);

	if (!CErrorManager::Instance().IsOk())
		CErrorManager::Instance().FlushErrors();

	//ant tweak bar destroy
	TwTerminate();

	exit(EXIT_SUCCESS);
}

/*
* Move selected control points
*/
void MoveControlPoints(int _x,int _y){
	if (selecionado[2]!=0){
		float x,y;
		m_coordinateSystemHelper.ScreenToImage(_x, glutGet(GLUT_WINDOW_HEIGHT) - _y, &x, &y);

		if (selecionado[2]==1){
		int *pontosControle = m_currentContour->GetControlPointCoordArray();
			pontosControle[selecionado[0]*2] = x;
			pontosControle[selecionado[0]*2+1] = y;
		} else{
			int *pontosControleParaCurve = m_currentContour->GetParaCurvePointCoordArray(selecionado[0]);
			pontosControleParaCurve[selecionado[1]*2] = x;
			pontosControleParaCurve[selecionado[1] * 2 + 1] = y;
		}
	}
}

/**
 * Rendering callback. It is designed only to show 2D slices.
 */
void OnPaint(void)
{
	int i;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	m_camera->ApplyTransform();

	glColor3f(1.0f, 1.0f, 1.0f);

	if ( show_text ) 
	{
		glRasterPos2i( 200, 50 );

		char *string2render = { "LOADING ... " };
		for(int i=0; i<(int)strlen( string2render ); i++ )
		  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, string2render[i] );
	}
    
	if (firstTime)
	{
		// Draw tweak bars
		TwDraw();
		glutSwapBuffers();
		return;
	}
	
	// The slice is displayed as a quad texture (presentation and data layers 
	// attached to improve overall performance).
	m_currentSlice->GetSliceTexture()->Bind(0);

	glBegin(GL_QUADS);

	//Puts the dataset upside down
	//glTexCoord2f(0.0f, 1.0f);
	//glVertex2i(0, 0);		
	//glTexCoord2f(1.0f, 1.0f);
	//glVertex2i(m_currentSlice->GetWidth(), 0);
	//glTexCoord2f(1.0f, 0.0f);
	//glVertex2i(m_currentSlice->GetWidth(), m_currentSlice->GetHeight());	
	//glTexCoord2f(0.0f, 0.0f);
	//glVertex2i(0, m_currentSlice->GetHeight());

	
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(0, 0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(m_currentSlice->GetWidth(), 0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(m_currentSlice->GetWidth(), m_currentSlice->GetHeight());
	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(0, m_currentSlice->GetHeight());


	glEnd();

	CTexture::DisableTextureUnit(0);
	
	glLineWidth(3.0f);
	
	// The coordinates of contour points are retrieved to display the contour.
	if (m_currentContour.get())
		if (m_currentContour->IsStarted())
		{
			glEnable(GL_VERTEX_ARRAY);
			
			glColor3f(1.0f, 1.0f, 0.0f);
			
			const int *pontosControle = m_currentContour->GetControlPointCoordArray();
			glBegin(GL_LINE_STRIP);			
			for (i=0; i<m_currentContour->GetControlPointCount(); i++)
			{

				const int *pontosParaCurve = m_currentContour->GetParaCurvePointCoordArray(i);
				if (i!=0){		
					int pOrigem[2];
					int pDestino[2];
					float delta=0.1;
					pOrigem[0] = pontosControle[i*2-2];
					pOrigem[1] = pontosControle[i*2-1];

					glVertex3f(pOrigem[0],pOrigem[1],0);

					for (float t=0; t<=1 ; t+=delta)
					{
						pDestino[0]= ( pOrigem[0] * ((1 - t) * (1 - t) * (1 - t))) + 
							( pontosParaCurve[0] * (3 * t * ((1 - t) * (1 - t)))) + 
							( pontosParaCurve[2] * (3 * (t * t) * (1 - t))) + 
							( pontosControle[i*2] * (t*t*t));
						pDestino[1]= ( pOrigem[1] * ((1 - t) * (1 - t) * (1 - t))) + 
							( pontosParaCurve[1] * (3 * t * ((1 - t) * (1 - t)))) + 
							( pontosParaCurve[3] * (3 * (t * t) * (1 - t))) + 
							( pontosControle[i*2+1] * (t*t*t));

						glColor3f(1,1,0);

						glVertex3f(pDestino[0],pDestino[1],0);

						pOrigem[0] = pDestino[0];
						pOrigem[1] = pDestino[1];
					}
				glVertex3f(pontosControle[i*2],pontosControle[i*2+1],0);
				}
			}
			glEnd();

			glPointSize(5.0f);

			glColor3f(0.0f, 0.0f, 1.0f);

			for (i=0; i<m_currentContour->GetControlPointCount(); i++)
			{
				glVertexPointer(2, GL_INT, 0, m_currentContour->GetParaCurvePointCoordArray(i));

				glDrawArrays(GL_POINTS, 0, m_currentContour->GetParaCurvePointCoordCount(i));

			}	

			glVertexPointer(2, GL_INT, 0, m_currentContour->GetControlPointCoordArray());

			glPointSize(6.0f);

			glColor3f(0.0f, 0.0f, 1.0f);

			glDrawArrays(GL_POINTS, 0, m_currentContour->GetControlPointCount());

			glPointSize(4.0f);

			glColor3f(1.0f, 1.0f, 1.0f);

			glDrawArrays(GL_POINTS, 0, m_currentContour->GetControlPointCount());

			
			glPointSize(6.0f);
			glColor3f(0.0f, 1.0f, 0.0f);
			glBegin(GL_POINTS);
			if (selecionado[2]!=0){
				if (selecionado[2]==1){
					glVertex2f(pontosControle[selecionado[0]*2],pontosControle[selecionado[0]*2+1]);
				
				}else{
					const int *pontosParaCurve = m_currentContour->GetParaCurvePointCoordArray(selecionado[0]);
					glVertex2f(pontosParaCurve[selecionado[1]*2],pontosParaCurve[selecionado[1]*2+1]);
				}
			}

			glEnd();

			glPointSize(3.0f);

			if (m_currentContour->HaveCandidatePoints() && (GetApplicationMode() == LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR))
			{

				glVertexPointer(2, GL_INT, 0, m_currentContour->GetCandidatePointCoordArray());

				glDrawArrays(GL_LINE_STRIP, 0, m_currentContour->GetCandidatePointCount());
				glEnable(GL_VERTEX_ARRAY);

				glColor3f(1.0f, 0.0f, 0.0f);
			
				const int *pontosControleC = m_currentContour->GetControlPointCoordArray();

				glBegin(GL_LINE_STRIP);		
					const int *pontosParaCurveC = m_currentContour->GetCandidatePointCoordArray();	
					int pOrigem[2];
					int pDestino[2];
					float delta=0.1;
					pOrigem[0] = pontosControleC[m_currentContour->GetControlPointCount()*2-2];
					pOrigem[1] = pontosControleC[m_currentContour->GetControlPointCount()*2-1];
					int aux = m_currentContour->GetCandidatePointCount()*2/4;
					if (aux % 2 != 0){
						aux++;
					}
					for (float t=0; t<=1 ; t+=delta)
					{
						pDestino[0]= ( pOrigem[0] * ((1 - t) * (1 - t) * (1 - t))) + 
							( pontosParaCurveC[aux*3] * (3 * t * ((1 - t) * (1 - t)))) + 
							( pontosParaCurveC[aux] * (3 * (t * t) * (1 - t))) + 
							( pontosParaCurveC[0] * (t*t*t));
						pDestino[1]= ( pOrigem[1] * ((1 - t) * (1 - t) * (1 - t))) + 
							( pontosParaCurveC[aux*3+1] * (3 * t * ((1 - t) * (1 - t)))) + 
							( pontosParaCurveC[aux+1] * (3 * (t * t) * (1 - t))) + 
							( pontosParaCurveC[1] * (t*t*t));

						glVertex3f(pOrigem[0],pOrigem[1],0);
						glVertex3f(pDestino[0],pDestino[1],0);

						pOrigem[0] = pDestino[0];
						pOrigem[1] = pDestino[1];
					}
				glVertex3f(pontosParaCurveC[0],pontosParaCurveC[1],0);
				glEnd();	
			}

			glDisable(GL_VERTEX_ARRAY);
		}
	
		if (GetApplicationMode()==BOUNDINGBOXMODE){ //***************************************************************************** boundingBox
			float xF, yF;
			m_coordinateSystemHelper.ScreenToImage(m_currentMousePositionX, glutGet(GLUT_WINDOW_HEIGHT) - m_currentMousePositionY, &xF, &yF);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f (0.9,0.9,0.9,0.5);
			glBegin(GL_QUADS);


			glVertex2f(boundingBox[0], boundingBox[2]);
			if(boundingBox[0]!=boundingBox[1] && boundingBox[2]!=boundingBox[3]){
				glVertex2f(xF, boundingBox[2]);
				glVertex2f(xF,yF);
				glVertex2f(boundingBox[0], yF);
			}
			else{
				glVertex2f(boundingBox[1], boundingBox[2]);
				glVertex2f(boundingBox[1],boundingBox[3]);
				glVertex2f(boundingBox[0], boundingBox[3]);
			}

			glEnd();
			glDisable(GL_BLEND);
		}
		else 
			if (WorkspaceIsDefined && show_workspace){
				glColor3f (1.0,0.5,0.0);
				glBegin(GL_LINE_STRIP);
					glVertex2f(boundingBox[0], boundingBox[2]);
					glVertex2f(boundingBox[1], boundingBox[2]);
					glVertex2f(boundingBox[1],boundingBox[3]);
					glVertex2f(boundingBox[0], boundingBox[3]);
					glVertex2f(boundingBox[0], boundingBox[2]);
				glEnd();
			}
		
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0, 100.0, 0.0, 100.0  );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glColor3ub( 255, 255, 255 );
	glRasterPos2i( 2, 2 );

	if ( m_applicationMode == BOUNDINGBOXMODE )
	{
		glRasterPos2i( 30, 6 );
		char *string2render = { "DEFINE THE WORKSPACE" };
		for(int i=0; i<(int)strlen( string2render ); i++ )
		  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, string2render[i] );	
		glRasterPos2i( 30, 2 );
		char *string2render2 = { "PRESS ENTER WHEN DONE/SET" };
		for(int i=0; i<(int)strlen( string2render2 ); i++ )
		  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, string2render2[i] );	
	}

	char buff[10];
	glRasterPos2i( 2, 6 );
	string wl = "W "; 
	wl+=itoa (m_currentSlice->GetWindowWidth(), buff,10);
	wl += " : L ";
	wl+=itoa (m_currentSlice->GetWindowCenter(), buff,10);	
	for( int i=0; i<wl.length(); i++ )
	  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, wl[i] );

	// Draw tweak bars
	TwDraw();

    glutSwapBuffers();
}

/**
 * Resize window
 */
void OnSize(GLsizei w, GLsizei h)
{
    if (h == 0)
        h = 1;
    
    glViewport(0, 0, w, h);

	m_camera->SetViewport(0, 0, w, h);

	TwWindowSize(w, h); //tweak bar
}

/**
 * Set interaction mode
 */
void setMode( int control )
{
	show_text = true;
	OnPaint();
    switch (control)
    {
	case BOUNDINGBOX_ID:

		break;

	case OPEN_DICOM_FILE:
		break;

	case OPEN_RAW:		
		if (!LoadRaw())
		{
			MarkError();
			OnDestroy();
		}
		break;

	case OPEN_DICOM_FOLDER:
		if (!LoadDicomFolder())
		{
			MarkError();
			OnDestroy();
		}
		break;

	case REGULAGEM_NAV_ID:
		if (!SetApplicationMode(LBUTTON2PAN_RBUTTON2ZOOM))
		{
			MarkError();
			OnDestroy();
		}
		break;
	case CONTOUR_ADD_REMOVE_ID:
		if (!SetApplicationMode(LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR))
		{
			MarkError();
			OnDestroy();
		}
		break;
	case REGULAGEM_JANELA_ID:
		if (!SetApplicationMode(LBUTTON2WINDOW))
		{
			MarkError();
			OnDestroy();
		}
		break;
	case CONTOUR_EDIT_ID:
		if (!SetApplicationMode(EDITCONTOURMODE))
		{
			MarkError();
			OnDestroy();
		}
		break;
	case SLICE_FORWARD_ID:
		if (!MoveCurrentSliceBackward())
		{
			MarkError();
			OnDestroy();
		}
		break;
	case SLICE_BACKWARD_ID:
		if (!MoveCurrentSliceForward())
		{
			MarkError();
			OnDestroy();
		}
		break;
	case CLOSE_CONTOUR_ID:
		if (!CloseCurrentContour())
		{
			MarkError();
			OnDestroy();
		}
		break;
	case COPY_CONTOUR_ID:
		if (!CopyCurrentContour())
		{
			MarkError();
			OnDestroy();
		}
		break;
	case CLEAN_CONTOUR:
		m_currentContour->Clear();
		break;
	case PASTE_CONTOUR_ID:
		if (!PasteCurrentContour())
		{
			MarkError();
			OnDestroy();
		}
		break;
	case EXPAND_CONTROL_POINTS:
		if (!ExpandControlPoints())
		{
			MarkError();
			OnDestroy();
		}
		break;
	case EXPAND_PARA_CURVE_POINTS:
		if (!ExpandParaCurvePoints())
		{
			MarkError();
			OnDestroy();
		}
		break;
	case CMD_CLOSE_ID:
			OnDestroy();        
			exit(EXIT_SUCCESS);
		break;
	}

	show_text = false;
		
}

/*
*Seleciona um ponto de controle ou ponto da curva paramétrica
*de acordo com o clique do mouse
*/
void SelectControlPoints(int _x, int _y){
	float x,y;
	int abertura = 5;
	m_coordinateSystemHelper.ScreenToImage(_x, glutGet(GLUT_WINDOW_HEIGHT) - _y, &x, &y);	
	int xOrdenado[2]={x-abertura,x+abertura};
	int yOrdenado[2]={y-abertura,y+abertura};

	if (m_currentContour.get())
		if (m_currentContour->IsStarted())
		{
			const int *pontosControle = m_currentContour->GetControlPointCoordArray();
			
			for (int i=0; i<m_currentContour->GetControlPointCount(); i++)
			{
				const int *pontosControleParaCurve = m_currentContour->GetParaCurvePointCoordArray(i);
				for (int j=0; j<m_currentContour->GetParaCurvePointCoordCount(i); j++){
					if (pontosControleParaCurve[j*2]>= xOrdenado[0] && pontosControleParaCurve[j*2]<= xOrdenado[1]){
						if (pontosControleParaCurve[j*2+1]>= yOrdenado[0] && pontosControleParaCurve[j*2+1]<= yOrdenado[1]){
							selecionado[0]=i;
							selecionado[1]=j; 
							selecionado[2]=2;
						}
					}
				
				}
				if (pontosControle[i*2]>= xOrdenado[0] && pontosControle[i*2]<= xOrdenado[1]){
					if (pontosControle[i*2+1]>= yOrdenado[0] && pontosControle[i*2+1]<= yOrdenado[1]){
						selecionado[0]=i;
						selecionado[2]=1;
					}
				}
				
			}
		}
}


void OnIdle(void)
{
	if ( glutGetWindow() != main_window ) 
		glutSetWindow(main_window);  

	glutPostRedisplay();
}

/**
 *
 */
void OnVisible(int isVisible)
{
    if (isVisible == GLUT_VISIBLE)	
        glutIdleFunc(OnIdle);
    else 
        glutIdleFunc(0);
}

/**
 *
 */
void OnMouse(int button, int state, int x, int y)
{
	if (!TwEventMouseButtonGLUT(button, state, x, y))//process tweak bar interaction
	{
		if (state == GLUT_DOWN)
		{
			switch (GetApplicationMode())
			{
				case BOUNDINGBOXMODE:
					m_coordinateSystemHelper.ScreenToImage(x, glutGet(GLUT_WINDOW_HEIGHT) - y, &boundingBox[0], &boundingBox[2]);
					definingWorkspace=true;
					break;
				case LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR:
					if (button == GLUT_LEFT_BUTTON)
					{
						if (!AddControlPointToCurrentContour(x, y))
						{
							MarkError();

							OnDestroy();
						}
					}

					else if (button == GLUT_RIGHT_BUTTON)
						if (!RemoveLastControlPointOfCurrentContour())
						{
							MarkError();
							
							OnDestroy();
						}
					break;
				case EDITCONTOURMODE:
					if (button == GLUT_LEFT_BUTTON)
					{

						SelectControlPoints(x,y);
					}
					break;
			}
			m_currentMouseButton = button;
		}
		else
			m_currentMouseButton = 0;

		if (state == GLUT_UP)
		{

			switch (GetApplicationMode())
			{
				case BOUNDINGBOXMODE:
					if (definingWorkspace)
						m_coordinateSystemHelper.ScreenToImage(x, glutGet(GLUT_WINDOW_HEIGHT) - y, &boundingBox[1], &boundingBox[3]);	
					definingWorkspace=false;
					break;
				case EDITCONTOURMODE:
					if (button == GLUT_LEFT_BUTTON)
						selecionado[2]=0;
					break;
			}
			m_currentMouseButton = button;
		}
			
		m_currentMousePositionX = x;
		m_currentMousePositionY = y;
	}
    glutPostRedisplay();
}

/**
 *
 */
void OnMouseMove(int x, int y)
{
	if( !TwEventMouseMotionGLUT(x, y) ) //process tweak bar interaction
	{
		switch (m_currentMouseButton)
		{
		case GLUT_LEFT_BUTTON:
			switch (GetApplicationMode())
			{
			case LBUTTON2PAN_RBUTTON2ZOOM:
				m_camera->MoveSide(0.25*(x - m_currentMousePositionX));
				m_camera->MoveUp(0.25f*(m_currentMousePositionY - y));
				break;
			case LBUTTON2WINDOW:
				if (!UpdateCurrentSliceWindow(x - m_currentMousePositionX, y - m_currentMousePositionY))
				{
					MarkError();
					OnDestroy();
				}
				break;
			case EDITCONTOURMODE:

				MoveControlPoints(x,y);

				break;
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			break;
		case GLUT_RIGHT_BUTTON:
			switch (GetApplicationMode())
			{
			case LBUTTON2PAN_RBUTTON2ZOOM:
				{
					float dS
						  , dT;

					dS = 0.005f*(y - m_currentMousePositionY)*(m_camera->GetRightClippingPlaneCoord() - m_camera->GetLeftClippingPlaneCoord());
					dT = 0.005f*(y - m_currentMousePositionY)*(m_camera->GetTopClippingPlaneCoord() - m_camera->GetBottomClippingPlaneCoord());

					if (!m_camera->Create(m_camera->GetLeftClippingPlaneCoord() - dS, m_camera->GetRightClippingPlaneCoord() + dS, m_camera->GetBottomClippingPlaneCoord() - dT, m_camera->GetTopClippingPlaneCoord() + dT, 0.0f, 1.0f, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)))
					{
						MarkError();
						OnDestroy();
					}
				}
				break;
			}
			break;
		}

		m_currentMousePositionX = x;
		m_currentMousePositionY = y;
	}
    glutPostRedisplay();
}

/**
 *
 */
void OnPassiveMouseMove(int x, int y)
{
	if( !TwEventMouseMotionGLUT(x, y) ) //process tweak bar interaction
		if (GetApplicationMode() == LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR)
			if (!UpdateNextControlPointOfCurrentContour(x, y))
			{
				MarkError();
				OnDestroy();
			}

    glutPostRedisplay();
}

/**
*
*/
void preprocessLiveWire()
{
	int slice_position = m_currentSlice->GetSliceLocation();

	//progressBar->setRange(1, m_depth);
	//progressBar->reset();
	//progressBar->show();

	//int p = progressBar->value();	

	for (int i = 0 ; i < m_depth ; i++)
	{
		if (!MoveCurrentSliceBackward())
		{
			MarkError();
			OnDestroy();
		}
		//progressBar->setValue( ++p );
	}
	for (int i = 0 ; i < m_depth ; i++)
	{
		if (!MoveCurrentSliceForward())
		{
			MarkError();
			OnDestroy();
		}
		//progressBar->setValue( ++p );
	}

	for (int i = 0 ; i < m_depth - slice_position ; i++){
		MoveCurrentSliceBackward();
	}

	//progressBar->hide();
}

/**
 *
 */
void OnKeyDown(unsigned char key, int x, int y) 
{
	if (!TwEventKeyboardGLUT(key, x, y))
	{
		if (( m_applicationMode = BOUNDINGBOXMODE) && (key == ENTER_KEY))	
		{
			WorkspaceIsDefined = true;
			setMode(CONTOUR_ADD_REMOVE_ID);	
			preprocessLiveWire();					
			return;
		}
		switch (key)
		{
		case '+':
			if (!MoveCurrentSliceForward())
			{
				MarkError();
				OnDestroy();
			}
			break;
		case '-':
			if (!MoveCurrentSliceBackward())
			{
				MarkError();
				OnDestroy();
			}
			break;
		case '1':
			if (!SetApplicationMode(LBUTTON2PAN_RBUTTON2ZOOM))
			{
				MarkError();
				OnDestroy();
			}
			break;
		case '2':
			if (!SetApplicationMode(LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR))
			{
				MarkError();
				OnDestroy();
			}
			break;
		case '3':
			if (!SetApplicationMode(LBUTTON2WINDOW))
			{
				MarkError();
				OnDestroy();
			}
			break;
		case '4':
			if (!SetApplicationMode(EDITCONTOURMODE))
			{
				MarkError();
				OnDestroy();
			}
			break;

		case 'c':
		case 'C':
			if (!CloseCurrentContour())
			{
				MarkError();
				OnDestroy();
			}
			break;
		}
	}
	glutPostRedisplay();
}

/******************************************************************************/
/****************   ANT TWEAK BAR FUNCTIONS   *********************************/
/******************************************************************************/

void TW_CALL OpenDicomCB(void* client)
{
	if (!LoadDicomFolder())
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL OpenRawCB(void* client)
{
    if (!LoadRaw())
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL FilterCB(void* client)
{	
	if (!Filter())
	{
		MarkError();
		OnDestroy();
	} 
}

void TW_CALL WindowCB(void* client)
{
    if (!SetApplicationMode(LBUTTON2WINDOW))
	{
		MarkError();
		OnDestroy();
	}                           
}

void TW_CALL PanCB(void* client)
{
    if (!SetApplicationMode(LBUTTON2PAN_RBUTTON2ZOOM))
	{
		MarkError();
		OnDestroy();
	}                           
}

void TW_CALL AddRemovePointCB(void* client)
{
    if (!SetApplicationMode(LBUTTON2ADDPOINTTOCONTOUR_RBUTTON2REMOVEPOINTFROMCONTOUR))
	{
		MarkError();
		OnDestroy();
	}                           
}

void TW_CALL EditPointCB(void* client)
{
	if (!SetApplicationMode(EDITCONTOURMODE))
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL CloseContourCB(void* client)
{
	if (!CloseCurrentContour())
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL CopyContourCB(void* client)
{
	if (!CopyCurrentContour())
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL PasteContourCB(void* client)
{
	if (!PasteCurrentContour())
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL EraseContourCB(void* client)
{
	m_currentContour->Clear();
}

void TW_CALL ExpandWhiteCB(void* client)
{
	if (!ExpandControlPoints())
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL ExpandBlueCB(void* client)
{
	if (!ExpandParaCurvePoints())
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL SaveContourCB(void* client)
{
	//save file window
	OPENFILENAME ofn ;	
	char szFile[255] ;

	SecureZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "RAW\0*.raw\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir = NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if (GetSaveFileName(&ofn))
		onSave(szFile);
}

void TW_CALL SavePointsCB(void* client)
{
	//save file window
	OPENFILENAME ofn ;	
	char szFile[255] ;

	SecureZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "DOTS\0*.dots\0All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if (GetSaveFileName(&ofn))
		onSaveContourPoints(szFile);
		
} 

void TW_CALL LoadPointsCB(void* client)
{
	if (WorkspaceIsDefined)
	{
		//openfile window
		OPENFILENAME ofn ;	
		char szFile[255] ;
		char type[255] = "DOTS\0*.dots\0All\0*.*\0";
		if (OpenFileWindow(szFile,type)){
			onLoadContourPoints(szFile);
		}			
	}
	else m_applicationMode = BOUNDINGBOXMODE;	
}

void TW_CALL setSlices(const void *value, void *clientData)
{ 
 while (slice != *(const int *)value)
 {
	 if (slice > *(const int *)value){
		MoveCurrentSliceForward();
		slice--;
	 }
	 if (slice < *(const int *)value){
		MoveCurrentSliceBackward();
		slice++;
	 }
	 //flush aqui para dar efeito de movimento
 }
}

void TW_CALL getSlices(void *value, void *clientData)
{ 
	*(int *)value = m_depth - m_currentSlice->GetSliceLocation() + 1;
}

void TW_CALL setWindow_W(const void *value, void *clientData)
{ 
	window_w = *(const int *)value;
	if (!m_currentSlice->UpdateWindow(window_l, window_w))
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL getWindow_W(void *value, void *clientData)
{ 
	*(int *)value = window_w;
}

void TW_CALL setWindow_L(const void *value, void *clientData)
{ 
	window_l = *(const int *)value;
	if (!m_currentSlice->UpdateWindow(window_l, window_w))
	{
		MarkError();
		OnDestroy();
	}
}

void TW_CALL getWindow_L(void *value, void *clientData)
{ 
	*(int *)value = window_l;
}

void TW_CALL WorskpaceCB(void* client)
{
	WorkspaceIsDefined = false;
	SetApplicationMode(BOUNDINGBOXMODE);
}

void TW_CALL ColorDefine(void* client)
{
	string definition;
	char buff[255];
	definition = " SmartContour color='";
	definition.append(itoa((int)(barColor[0]*255), buff, 10));
	definition.append(" ");
	definition.append(itoa((int)(barColor[1]*255), buff, 10));
	definition.append(" ");
	definition.append(itoa((int)(barColor[2]*255), buff, 10));
	definition.append("'");
	TwDefine(definition.c_str());
}

void TW_CALL ExitCB(void* client)
{
	//deseja salvar?
	OnDestroy();
	exit(EXIT_SUCCESS);
}

// Function responsable for the creation of the menu
bool TweakBarCreate(int slices)
{
	if (bar!=NULL) TwDeleteBar(bar); //delete old bar

	// Create a tweak bar
    bar = TwNewBar("SmartContour");	 
	string definition =" SmartContour label='SmartContour' fontSize=3 position='0 0' size='270 250' valuesWidth=100 ";
	TwDefine(definition.c_str());
	ColorDefine(NULL);
	TwWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)); 	
    TwDefine(" GLOBAL help='Help' "); // Message added to the help bar.
    
	// FILE ----------------------------------------------------------------------------------------------------------
	
	TwAddButton(bar, "raw", OpenRawCB, NULL , 
                "  group='File' label='Open Images/Raw' help='Open raw file.' ");

	definition = " group='File' label='Convert Images/DICOM' help='Select a directory with DICOM files to open.";
	definition.append(" SmartContour will convert them to a single file with RAW extension. This take time. Thus the next time you want ");
	definition.append("to open this collection of images we recomend you use the Open Raw button and select the associated raw file.' ");
	
	TwAddButton(bar, "dicom", OpenDicomCB, NULL, definition.c_str());

	TwAddSeparator(bar, "", NULL);


	if (slices>0)
	{	
		TwDefine(" SmartContour size='270 700' ");
		
		TwAddButton(bar, "filter", FilterCB, NULL, " group='File' label='Apply Filter' ");

		//TwAddSeparator(bar, "", NULL);

		TwAddButton(bar, "savecontour", SaveContourCB, NULL, 
                " group='File' label='Save Contour' help='.' ");

		// NAVIGATION -----------------------------------------------------------------------------------------
		definition = " group='Navigation' label='Slice' min=1 max=";
		char s_slices[255];
		itoa(slices, s_slices, 10);
		definition.append(s_slices);
		definition.append("help='Slice position number' ");

		TwAddVarCB( bar, "slice", TW_TYPE_UINT16, setSlices, getSlices, NULL, definition.c_str());
		TwAddButton(bar, "window", WindowCB, NULL, 
                "  group='Navigation' label='Window' help='.' ");
	
		TwAddVarCB( bar, "W", TW_TYPE_UINT16, setWindow_W, getWindow_W, NULL, " group='Navigation' ");
		TwAddVarCB( bar, "L", TW_TYPE_UINT16, setWindow_L, getWindow_L, NULL, " group='Navigation' ");

		TwAddButton(bar, "pan", PanCB, NULL, 
                "  group='Navigation' label='Pan' help='.' ");

		TwAddSeparator(bar, "", NULL);

		// DISPLAY -----------------------------------------------------------------------------------------

		// >> Contour
		TwAddButton(bar, "close", CloseContourCB, NULL, 
                "  group='Contour' label='Close Contour' help='.' ");
		TwAddButton(bar, "copy", CopyContourCB, NULL, 
                "  group='Contour' label='Copy Contour' help='.' ");
		TwAddButton(bar, "paste", PasteContourCB, NULL, 
                "  group='Contour' label='Paste Contour' help='.' ");
		TwAddButton(bar, "erase", EraseContourCB, NULL, 
                "  group='Contour' label='Erase Contour' help='.' ");

		// >> Point
		TwAddButton(bar, "point", AddRemovePointCB , NULL,
                "  group='Point' label='ADD/Remove Point' help='.' ");
		TwAddButton(bar, "edit", EditPointCB, NULL, 
                "  group='Point' label='Edit Point' help='.' ");
		TwAddButton(bar, "white", ExpandWhiteCB, NULL, 
                "  group='Point' label='Expand White Points' help='EXPAND CONTROL POINTS.' ");
		TwAddButton(bar, "blue", ExpandBlueCB, NULL, 
                "  group='Point' label='Expand Blue Points' help='EXPAND CURVE POINTS.' ");
		TwAddButton(bar, "save", SavePointsCB, NULL, 
                "  group='Point' label='Save Points' help='.' ");
		TwAddButton(bar, "load", LoadPointsCB, NULL, 
                "  group='Point' label='Load Points' help='.' ");

		TwDefine(" SmartContour/Contour group='Display' ");  
		TwDefine(" SmartContour/Point group='Display' "); 
	
		TwAddSeparator(bar, "", NULL);



		// WORKSPACE ---------------------------------------------------------------------------------------
		//TwAddButton(bar, "workspace", WorskpaceCB, NULL, 
             //   "  group='Workspace' label='Define/Redefine Workspace' help='.' ");

		TwAddVarRW( bar, "show_workspace", TW_TYPE_BOOLCPP, &show_workspace, 
			" group='Workspace' label='Show Workspace'");


	}

	// OPTIONS -----------------------------------------------------------------------------------------

	TwAddVarRW(bar, "Color", TW_TYPE_COLOR3F, &barColor, " group='Options' ");
	TwAddButton(bar, "colorb", ColorDefine, NULL, 
                " label='OK' group='Color' ");
	
	TwAddSeparator(bar, "", NULL);

	// EXIT -----------------------------------------------------------------------------------------

	TwAddButton(bar, "exit", ExitCB, NULL, 
                " label='Exit' ");
	
	return true;

}

/************************************************************************************************
 * The initialization function.
 * Initializes GLUT, GLEW and Ant Tweak Bar
 ************************************************************************************************/
bool OnCreate(int argc, char** argv)
{	
	
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowPosition( 250, 50 );
	glutInitWindowSize(INI_WINDOW_WIDTH, INI_WINDOW_HEIGHT);

	main_window = glutCreateWindow( "SmartContour" );

	// Loads OpenGL extensions.
	if (glewInit() != GLEW_OK)
	{
		MarkError();
		return false;
	}

	glutDisplayFunc(OnPaint);
	glutReshapeFunc(OnSize);
	glutVisibilityFunc(OnVisible);
	glutIdleFunc(OnIdle);

	glutMouseFunc(OnMouse);
    glutMotionFunc(OnMouseMove);
    glutPassiveMotionFunc(OnPassiveMouseMove);
	glutKeyboardFunc(OnKeyDown);
	
	TwGLUTModifiersFunc(glutGetModifiers);

	boundingBox[0] = 0;
	boundingBox[1] = 0;
	boundingBox[2] = 0;
	boundingBox[3] = 0;		

	
	if( !TwInit(TW_OPENGL, NULL) )
    {
        // A fatal error occured    
        fprintf(stderr, "AntTweakBar initialization failed: %s\n", TwGetLastError());
        return false;
    }

	//progressBar = new QProgressBar();
	TweakBarCreate(0); //menu bar	
	
	return true;
}

/*
 * salva pontos do contorno em arquivo .dots (txt)
 */
void onSaveContourPoints(std::string fileSaveName){
	setMode(CONTOUR_ADD_REMOVE_ID);	
	m_contourArray->SaveContourPoints(fileSaveName);
}

/*
* carrega pontos do contorno em arquivo .dots (txt)
*/
void onLoadContourPoints(std::string fileLoadName){
	setMode(CONTOUR_ADD_REMOVE_ID);	
	m_contourArray->LoadContourPoints(fileLoadName);
}

void floodFill(short *imagem, int x, int y, int newColor,int oldColor){
	int h = m_height;
    int w = m_width;
	if(oldColor == newColor) return;
    if(imagem[y*w+x] != oldColor) return;

    int y1;
    
    //draw current scanline from start position to the top
    y1 = y;
    while(imagem[y1*w+x] == oldColor && y1 < h)
    {
        imagem[y1*w+x] = newColor;
        y1++;
    }    
    
    //draw current scanline from start position to the bottom
    y1 = y - 1;
	while( y1 >= 0 && imagem[y1*w+x] == oldColor)
	{
		imagem[y1*w+x] = newColor;
		y1--;
	}     
    //test for new scanlines to the left    
    y1 = y;
    while(imagem[y1*w+x] == newColor && y1 < h)
    {        
        if(x > 0 && imagem[y1*w+x - 1] == oldColor) 
        {
            floodFill(imagem,x - 1, y1, newColor, oldColor);
        } 
        y1++;                    
    }    
    y1 = y - 1;
    while(y1 >= 0 && imagem[y1*w+x] == newColor)
    {
        if(x > 0 && imagem[y1*w+x - 1] == oldColor) 
        {
            floodFill(imagem, x - 1, y1, newColor, oldColor);
        } 
        y1--;                    
    } 
    
    //test for new scanlines to the right              
    y1 = y;
    while(imagem[y1*w+x] == newColor && y1 < h)
    {
        if(x < w - 1 && imagem[y1*w+x + 1] == oldColor) 
        {
            floodFill(imagem, x + 1, y1, newColor, oldColor);
        }    
        y1++;                    
    }         
    y1 = y - 1;
    while(y1 >= 0 && imagem[y1*w+x] == newColor )
    {
        if(x < w - 1 && imagem[y1*w+x + 1] == oldColor) 
        {
            floodFill(imagem, x + 1, y1, newColor, oldColor);
        }    
        y1--;                    
    }       
}

/*
 * save segmentation in raw format
 */
bool onSave(string saveNamePointer){

	int saveRange[2];
	saveRange[0]= 0;
	saveRange[1]= m_depth-1;
	if (saveRange[0]==saveRange[1])
		return false;
	if (saveRange[0] > saveRange[1])
	{
		saveRange[0] = saveRange[0] ^saveRange[1];
		saveRange[1] = saveRange[0] ^saveRange[1];
		saveRange[0] = saveRange[0] ^saveRange[1];
	}

	int fatias;
	fatias = saveRange[1]-saveRange[0]+1;

	int resolucao[2];
	resolucao[0] = m_width;
	resolucao[1] = m_height;

	short **imageSegmentada;
	//alloc memory for imageSegmentada
	imageSegmentada = (short**)malloc(fatias * sizeof(short*));
	for (int i=0; i < fatias; i++)
		imageSegmentada[i] = (short*)malloc(sizeof(short) * (resolucao[0])*(resolucao[1]));


	std::vector<boost::shared_ptr<CMyContour> > countorArray;
	m_contourArray->GetContourArray(countorArray);


	int res = resolucao[0]*resolucao[1];
	for( int i = 0; i < fatias; i++ ){
		for( int j = 0; j < res; j++ ){
			imageSegmentada[i][j] = 255;
		}
	}


	for (int k=saveRange[0]; k<=saveRange[1] ; k++){
			const int  *pontosControle = countorArray[k]->GetControlPointCoordArray();

			for (int i=0; i<countorArray[k]->GetControlPointCount(); i++)
			{
				const int *pontosParaCurve = countorArray[k]->GetParaCurvePointCoordArray(i);
				if (i!=0){		
					int pOrigem[2];
					int pDestino[2];
					float delta=0.001;
					pOrigem[0] = pontosControle[i*2-2];
					pOrigem[1] = pontosControle[i*2-1];
					for (float t=0; t<=1 ; t+=delta)
					{
						pDestino[0]= ( pOrigem[0] * ((1 - t) * (1 - t) * (1 - t))) + 
							( pontosParaCurve[0] * (3 * t * ((1 - t) * (1 - t)))) + 
							( pontosParaCurve[2] * (3 * (t * t) * (1 - t))) + 
							( pontosControle[i*2] * (t*t*t));
						pDestino[1]= ( pOrigem[1] * ((1 - t) * (1 - t) * (1 - t))) + 
							( pontosParaCurve[1] * (3 * t * ((1 - t) * (1 - t)))) + 
							( pontosParaCurve[3] * (3 * (t * t) * (1 - t))) + 
							( pontosControle[i*2+1] * (t*t*t));

						if (pDestino[0]>=0 && pDestino[0]<resolucao[0] && pDestino[1]>=0 && pDestino[1]<resolucao[1])
							imageSegmentada[k-saveRange[0]][(resolucao[1]*pDestino[0]+pDestino[1])]=0;

					}
				}
			}
	}
	//painting the empty space!!
	for( int i = 0; i < fatias; i++ ){
			floodFill(imageSegmentada[i],0,0,1,255);
	}

	//save segmentation file
	FILE* shortFile;
	string saveName = saveNamePointer+".raw";
	if( shortFile = fopen( saveName.c_str(), "wb+" ) ){
		for (int k=0 ; k < fatias ; k++){
			for (int i=0; i<m_width;i++){
				for (int j=0 ; j < m_height ; j++){
					fwrite( &imageSegmentada[k][(m_width*j+i)], 1, sizeof(short), shortFile );					
				}
			}
		}
		fclose(shortFile);
	}

	//save header
	CreateNhdrFile(saveNamePointer);

	return true;
}

/**
 *
 */
int main(int argc, char **argv)
{
	//QApplication app( argc, argv );

	if (!OnCreate(argc, argv))
	{
		MarkError();
		OnDestroy();
	}

	if (!InitializeCamera())
	{
		MarkError();
		OnDestroy();
	}

    glutMainLoop();
    
    return EXIT_SUCCESS;
}