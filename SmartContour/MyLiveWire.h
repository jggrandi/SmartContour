#ifndef LIVEWIRE_INCLUDED
#define LIVEWIRE_INCLUDED

#include <utility>
#include <vector>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <ErrorManager.h>

#include "IImage.h"

struct _Limits{
	int xMin;
	int xMax;
	int yMin;
	int yMax;
};

class CMyLiveWire
	: boost::noncopyable
{
	typedef std::pair<int, int>                                                                                    MyGraphArcType;
	typedef boost::property<boost::edge_weight_t, float>                                                           MyArcProperty;
	typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, boost::no_property, MyArcProperty > MyGraphType;
	typedef boost::graph_traits<MyGraphType>::vertex_descriptor                                                    MyVertexDescriptor;
	typedef boost::graph_traits<MyGraphType>::edge_descriptor                                                      MyArcDescriptor;
public:
	CMyLiveWire();
	virtual ~CMyLiveWire();
	bool LoadFromImage(boost::shared_ptr<IImage> image, int minX = 0 , int maxX = 0 , int minY = 0 , int maxY = 0);
	bool AddSeed(int x, int y);
	boost::shared_ptr<std::vector<int> > GetShortestPathFromSeed(int x, int y);
	bool IsValid() const;
	void SetLimits(int xMin, int xMax, int yMin, int yMax);
private:
	void Create(void);
	bool InitializeGraph(boost::shared_ptr<IImage> image);
	bool InitializeGraphArcs(boost::shared_ptr<IImage> image, boost::shared_ptr<std::vector<MyGraphArcType> > arcArray, boost::shared_ptr<std::vector<float> > arcWeightArray);
	bool UpdateGraphArcs(boost::shared_ptr<IImage> image);
	float FilterPixelValue(float referencePixelValue, float pixelValue) const;
	int GetNodeId(int x, int y) const;
	int LimitsIndexToImageIndex(int vIndex) const;


protected:
	int                                  m_imageWidth;
	int                                  m_imageHeight;
	boost::shared_ptr<MyGraphType>       m_imageGraph;
	// Descriptor of the seed vertex of the search.
	MyVertexDescriptor                   m_seedVertexDescriptor;
	std::vector<MyVertexDescriptor>      m_parentNodeIndexArray;
	std::vector<float>                   m_distanceToSeedArray;
	bool                                 m_hasSeed;
	boost::shared_ptr<std::vector<int> > m_shortestPathVertexIndexArray;
//	bool								 m_hasLimits;
	int									 m_liveWireWidth;
	int									 m_liveWireHeight;
	_Limits								 m_liveWireLimits;

};

#endif //LIVEWIRE_INCLUDED

