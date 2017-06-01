#include "ISeriesViewer.h"

/**
*/
ISeriesViewer::ISeriesViewer(void)
{
	Create();
}

/**
*/
ISeriesViewer::~ISeriesViewer(void)
{
}

/**
*/
int ISeriesViewer::GetWindowCenter(void) const
{
	return m_windowCenter;
}

/**
*/
int ISeriesViewer::GetWindowWidth(void) const
{
	return m_windowWidth;
}

/**
*/
void ISeriesViewer::Create(void)
{
	m_windowCenter = 400;
	m_windowWidth = 2000;
}

