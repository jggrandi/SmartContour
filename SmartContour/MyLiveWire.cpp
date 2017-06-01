#include <vector>

#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <ErrorManager.h>
#include <MyMath.h>

#include "MyLiveWire.h"

using namespace std;

/**
 *
 */
CMyLiveWire::CMyLiveWire()
{
	Create();
}

/**
 *
 */
CMyLiveWire::~CMyLiveWire()
{
}



/**
 *
 */
bool CMyLiveWire::LoadFromImage(boost::shared_ptr<IImage> image, int minX , int maxX , int minY , int maxY )
{
	
	if (!image.get())
	{
		MarkError();

		return false;
	}
	
	if (minX >= maxX || minY >= maxY){
		minX = 1;
		maxX = image->GetWidth()-2;
		minY = 1;
		maxY = image->GetHeight()-2;
	}

	SetLimits(minX,maxX,minY,maxY);

	if (!image->IsValid())
	{
		MarkError();

		return false;
	}

	if (!InitializeGraph(image))
	{
		MarkError();

		return false;
	}

	m_liveWireWidth = m_liveWireLimits.xMax - m_liveWireLimits.xMin+1;
	m_liveWireHeight = m_liveWireLimits.yMax - m_liveWireLimits.yMin+1;
	m_imageWidth = image->GetWidth();

	//m_imageWidth = image->GetWidth();
	//m_imageHeight = image->GetHeight();

	return true;
}

/**
*/
bool CMyLiveWire::AddSeed(int x, int y)
{
	int vertexIndex;
	
	x = x - m_liveWireLimits.xMin;
	y = y - m_liveWireLimits.yMin;
	vertexIndex = GetNodeId(x, y);

	m_seedVertexDescriptor = boost::vertex(vertexIndex, *m_imageGraph);

	boost::dijkstra_shortest_paths(*m_imageGraph, m_seedVertexDescriptor, boost::predecessor_map(&m_parentNodeIndexArray[0]).distance_map(&m_distanceToSeedArray[0]));

	m_hasSeed = true;

	return true;
}

/**
*/
boost::shared_ptr<std::vector<int> > CMyLiveWire::GetShortestPathFromSeed(int x, int y)
{
	int currentVertexIndex;
		
	x = x - m_liveWireLimits.xMin;
	y = y - m_liveWireLimits.yMin;

	currentVertexIndex = GetNodeId(x, y);

	m_shortestPathVertexIndexArray->clear();

	while (m_parentNodeIndexArray[currentVertexIndex] != m_seedVertexDescriptor)
	{

		if (m_parentNodeIndexArray[currentVertexIndex] == currentVertexIndex)
		{
			m_shortestPathVertexIndexArray->clear();

			break;
		}
		

		m_shortestPathVertexIndexArray->push_back(LimitsIndexToImageIndex(currentVertexIndex));

		//m_shortestPathVertexIndexArray->push_back(currentVertexIndex);

		currentVertexIndex = int(m_parentNodeIndexArray[currentVertexIndex]);
	}

	return m_shortestPathVertexIndexArray;
}

/**
*/
bool CMyLiveWire::IsValid() const
{
	return true;
}

/**
 *
 */
void CMyLiveWire::Create()
{
	m_imageWidth = 0;
	m_imageHeight = 0;

	m_imageGraph.reset();

	//m_seedVertexDescriptor?

	m_parentNodeIndexArray.clear();

	m_distanceToSeedArray.clear();

	m_hasSeed = false;

	m_shortestPathVertexIndexArray.reset(new vector<int>());

	m_shortestPathVertexIndexArray->clear();

	m_liveWireLimits.xMin = 0;
	m_liveWireLimits.xMax = 0;
	m_liveWireLimits.yMin = 0;
	m_liveWireLimits.yMax = 0;
}

/**
 *
 */
bool CMyLiveWire::InitializeGraph(boost::shared_ptr<IImage> image)
{
	int                                        imageWidth,
		                                       imageHeight,
											   numberOfNodes;
	boost::shared_ptr<vector<MyGraphArcType> > arcArray;
	boost::shared_ptr<vector<float> >          arcWeightArray;

	imageWidth = m_liveWireLimits.xMax-m_liveWireLimits.xMin+1;
	imageHeight = m_liveWireLimits.yMax-m_liveWireLimits.yMin+1;	
	
	//imageWidth = image->GetWidth();
	//imageHeight = image->GetHeight();	

	numberOfNodes = imageWidth*imageHeight;

	arcArray.reset(new vector<MyGraphArcType>());

	if (!arcArray.get())
	{
		MarkError();
		
		return false;
	}

	arcWeightArray.reset(new vector<float>());;

	if (!arcWeightArray.get())
	{
		MarkError();

		return false;
	}

	if (!InitializeGraphArcs(image, arcArray, arcWeightArray))
	{
		MarkError();

		return false;
	}

	m_imageGraph.reset(new MyGraphType(&(*arcArray)[0], &(*arcArray)[0] + arcArray->size(), &(*arcWeightArray)[0], numberOfNodes));

	if (!UpdateGraphArcs(image))
	{
		MarkError();

		return false;
	}

	m_parentNodeIndexArray.assign(numberOfNodes, MyVertexDescriptor());
	
	m_distanceToSeedArray.assign(numberOfNodes, 0.0f);

	return true;
}

/**
*/
int CMyLiveWire::GetNodeId(int x, int y) const
{
	if (x < 1)
		x = 2;
	
	else if (x >= (m_liveWireWidth - 1))
		x = m_liveWireWidth - 1 - 1;

	if (y < 1)
		y = 2;
	else if (y >= (m_liveWireHeight - 1))
		y = m_liveWireHeight - 1 - 1;

	return y * m_liveWireWidth + x;

	/*	if (x < 1)
		x = 1;
	else if (x >= (m_imageWidth - 1))
		x = m_imageWidth - 1 - 1;

	if (y < 1)
		y = 1;
	else if (y >= (m_imageHeight - 1))
		y = m_imageHeight - 1 - 1;

	return y*m_imageWidth + x;*/
}

/**
 *
 */
void CMyLiveWire::SetLimits(int xMin, int xMax, int yMin, int yMax)
{
	m_liveWireLimits.xMin = xMin;
	m_liveWireLimits.xMax = xMax;
	m_liveWireLimits.yMin = yMin;
	m_liveWireLimits.yMax = yMax;
}

int CMyLiveWire::LimitsIndexToImageIndex(int vIndex) const
{
	int auxDivInt = vIndex/m_liveWireWidth;
	int auxResto = vIndex%m_liveWireWidth;
	auxDivInt += m_liveWireLimits.yMin;
	auxResto += m_liveWireLimits.xMin;
	return auxDivInt * m_imageWidth + auxResto;
}

/**
*/
bool CMyLiveWire::UpdateGraphArcs(boost::shared_ptr<IImage> image)
{
	boost::property_map<MyGraphType, boost::edge_weight_t>::type weightMap;
	boost::graph_traits<MyGraphType>::edge_iterator arc, 
		arcEnd;
	MyArcDescriptor arcDescriptor;
	vector<float> imageSecondDerivativeMagnitude,
		imageLocalMaximum;
	int width,
		height,
		pixelIndex,
		sourcePixelIndex,
		targetPixelIndex,
		i,
		j;
	float pixelValue,
		nPixelValue,
		nePixelValue,
		ePixelValue,
		sePixelValue,
		sPixelValue,
		swPixelValue,
		wPixelValue,
		nwPixelValue,
		sourcePixelSecondDerivativeMagnitude,
		targetPixelSecondDerivativeMagnitude,
		sourcePixelLocalMaximum,
		targetPixelLocalMaximum,
		arcWeight,
		arcWeightMin,
		arcWeightMax;

	if (!m_imageGraph.get())
	{
		MarkError();

		return false;
	}

	
	//width = m_liveWireLimits.xMax-m_liveWireLimits.xMin;
	//height = m_liveWireLimits.yMax-m_liveWireLimits.yMin;	

	width = image->GetWidth();
	height = image->GetHeight();	

	int liveWireLimits=0;

	imageSecondDerivativeMagnitude.assign(width*height, 0.0f);

	imageLocalMaximum.assign(width*height, 0.0f);

	for (i=1; i<(height - 1); ++i)
		for (j=1; j<(width - 1); ++j)
		{
			if (j >= m_liveWireLimits.xMin && j <= m_liveWireLimits.xMax && i >= m_liveWireLimits.yMin && i <= m_liveWireLimits.yMax)
			{
			liveWireLimits++;

			pixelIndex = i * width + j;

			pixelValue = image->GetValue(pixelIndex);

			nPixelValue = image->GetValue(pixelIndex + width);
			nePixelValue = image->GetValue(pixelIndex + width + 1);
			ePixelValue = image->GetValue(pixelIndex + 1);
			sePixelValue = image->GetValue(pixelIndex - width + 1);
			sPixelValue = image->GetValue(pixelIndex - width);
			swPixelValue = image->GetValue(pixelIndex - width - 1);
			wPixelValue = image->GetValue(pixelIndex - 1);
			nwPixelValue = image->GetValue(pixelIndex + width - 1);

			// Basic second derivative filter.
			// 4-connected.
			//imageSecondDerivativeMagnitude[pixelIndex] = wPixelValue + nPixelValue + ePixelValue + sPixelValue - 4.0f*pixelValue;
			// 8-connected.
			//imageSecondDerivativeMagnitude[pixelIndex] = wPixelValue + nwPixelValue + nPixelValue + nePixelValue + ePixelValue + sePixelValue + sPixelValue +swPixelValue - 8.0f*pixelValue;

			// ?
			// 8-connected.
			//imageLocalMaximum[pixelIndex] = pixelValue - max(nPixelValue, max(nePixelValue, max(ePixelValue, max(sePixelValue, max(sPixelValue, max(swPixelValue, max(wPixelValue, nwPixelValue)))))));

			// 8-connected first derivative magnitude.
			imageSecondDerivativeMagnitude[liveWireLimits] = -fabs(
			//imageSecondDerivativeMagnitude[pixelIndex] = -fabs(
				-nwPixelValue + nePixelValue + 
				-2.0f*wPixelValue + 2.0f*ePixelValue + 
				-sePixelValue + swPixelValue) - fabs(
				-nwPixelValue -2.0f*nPixelValue -nePixelValue + 
				sePixelValue + 2.0f*sPixelValue + swPixelValue);
			}
		}

	weightMap = boost::get(boost::edge_weight, *m_imageGraph);

	arcWeightMin = MyMath::GetMaximumPositiveValue<float>();
	arcWeightMax = -MyMath::GetMaximumPositiveValue<float>();

	for (tie(arc, arcEnd)=boost::edges(*m_imageGraph); arc!=arcEnd; ++arc) 
	{
		arcDescriptor = *arc;

		sourcePixelIndex = int(boost::source(arcDescriptor, *m_imageGraph));
		targetPixelIndex = int(boost::target(arcDescriptor, *m_imageGraph));

		sourcePixelSecondDerivativeMagnitude = imageSecondDerivativeMagnitude[sourcePixelIndex];
		targetPixelSecondDerivativeMagnitude = imageSecondDerivativeMagnitude[targetPixelIndex];

		sourcePixelLocalMaximum = imageLocalMaximum[sourcePixelIndex];
		targetPixelLocalMaximum = imageLocalMaximum[targetPixelIndex];

		arcWeight = sourcePixelSecondDerivativeMagnitude + targetPixelSecondDerivativeMagnitude + sourcePixelLocalMaximum + targetPixelLocalMaximum;

		arcWeightMin = min(arcWeightMin, arcWeight);
		arcWeightMax = max(arcWeightMax, arcWeight);

		boost::get(weightMap, arcDescriptor) = arcWeight;
	}


	// Hack to keep all weights above (or equal to) 0.
	for (tie(arc, arcEnd)=boost::edges(*m_imageGraph); arc!=arcEnd; ++arc) 
	{
		arcDescriptor = *arc;

		boost::get(weightMap, arcDescriptor) = (boost::get(weightMap, arcDescriptor) - arcWeightMin)/(arcWeightMax - arcWeightMin);
	}

	return true;
}

/**
*/
bool CMyLiveWire::InitializeGraphArcs(boost::shared_ptr<IImage> image, boost::shared_ptr<vector<MyGraphArcType> > arcArray, boost::shared_ptr<vector<float> > arcWeightArray)
{
	int width,
		height,
		pixelIndex,
		upPixelIndex,
		rightPixelIndex,
		downPixelIndex,
		leftPixelIndex,
		i,
		j;

	if (!arcArray.get())
	{
		MarkError();
		
		return false;
	}
	
	if (!arcWeightArray.get())
	{
		MarkError();

		return false;
	}
	
	width = m_liveWireLimits.xMax-m_liveWireLimits.xMin+1;
	height = m_liveWireLimits.yMax-m_liveWireLimits.yMin+1;	
	//width = image->GetWidth();
	//height = image->GetHeight();	

	arcArray->reserve(4*width*height);
	arcWeightArray->reserve(4*width*height);

	for (i=0; i<height; i++)
		for (j=0; j<width; j++)
		{

			pixelIndex = i* width + j;
			
			upPixelIndex = pixelIndex + width;
			rightPixelIndex = pixelIndex + 1;
			downPixelIndex = pixelIndex - width;
			leftPixelIndex = pixelIndex - 1;

			// The calculus of the weight of the bel depends on neighbors of 
			// the pixel, and thus we have to consider a border to select the 
			// pixels which casts arcs. The size of the border will change as 
			// result of the pixels which will be consulted to calculate the 
			// cost of each arc.
			if ((i > 1) && (i < (height - 1)))
				if (j > 1)
					arcArray->push_back(make_pair(pixelIndex, upPixelIndex));
			
			if (i > (1 + 1))
				if (j > 1)
					arcArray->push_back(make_pair(pixelIndex, downPixelIndex));

			if (i > 1)
				if ((j > 1) && (j < (width - 1)))
					arcArray->push_back(make_pair(pixelIndex, rightPixelIndex));

			if (i > (1 + 1))
				if (j > (1 + 1))
					arcArray->push_back(make_pair(pixelIndex, leftPixelIndex));
		}

	arcWeightArray->assign(arcArray->size(), 1.0f);

	return true;
}



