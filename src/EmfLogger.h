/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#ifndef EMFLOGGER_H
#define EMFLOGGER_H

#include <QList>
#include <QPainter>
#include <QRect> // also provides QSize, QPoint
#include <QString>
#include <QVariant>
#include <QDebug>

#include "EmfEnums.h"
#include "EmfHeader.h"
#include "EmfRecords.h"
#include "EmfOutput.h"

class QFile;

/**
   \file

   Contains definitions for an EMF debug output strategy
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace QEmf
{


/**
	Debug (text dump) output strategy for EMF Parser
*/
class EmfLogger : public AbstractOutput
{
public:
	EmfLogger(const QString& = "");
	~EmfLogger() override;

	void init( const Header *header ) override;
	void cleanup( const Header *header ) override;
	void eof() override;

	void createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
			quint8 red, quint8 green, quint8 blue, quint32 brushStyle, quint8 reserved ) override;
	void createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
				  quint8 green, quint8 blue, quint8 reserved,
				  quint32 BrushHatch ) override;
	void createMonoBrush( quint32 ihBrush, Bitmap *bitmap ) override;
	void createDibPatternBrushPT( quint32 ihBrush, Bitmap *bitmap ) override;
	void selectObject( quint32 ihObject ) override;
	void deleteObject( quint32 ihObject ) override;
	void arc( const QRect &box, const QPoint &start, const QPoint &end ) override;
	void chord( const QRect &box, const QPoint &start, const QPoint &end ) override;
	void pie( const QRect &box, const QPoint &start, const QPoint &end ) override;
	void ellipse( const QRect &box ) override;
	void rectangle( const QRect &box ) override;
	void setMapMode( quint32 mapMode ) override;
	void setMetaRgn() override;
	void setWindowOrgEx( const QPoint &origin ) override;
	void setWindowExtEx( const QSize &size ) override;
	void setViewportOrgEx( const QPoint &origin ) override;
	void setViewportExtEx( const QSize &size ) override;
	void beginPath() override;
	void closeFigure() override;
	void endPath() override;
	void setBkMode( quint32 backgroundMode ) override;
	void setPolyFillMode( quint32 polyFillMode ) override;
	void setLayout( quint32 layoutMode ) override;
	void extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW ) override;
	void setTextAlign( quint32 textAlignMode ) override;
	void setTextColor( quint8 red, quint8 green, quint8 blue,
			   quint8 reserved ) override;
	void setBkColor( quint8 red, quint8 green, quint8 blue,
					 quint8 reserved ) override;
	void setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved ) override;
	void modifyWorldTransform( quint32 mode, float M11, float M12,
				   float M21, float M22, float Dx, float Dy ) override;
	void setWorldTransform( float M11, float M12, float M21,
				float M22, float Dx, float Dy ) override;
	void extTextOut( const QRect &bounds, const EmrTextObject &textObject ) override;
	void moveToEx( qint32 x, qint32 y ) override;
	void saveDC() override;
	void restoreDC( qint32 savedDC ) override;
	void lineTo( const QPoint &finishPoint ) override;
	void arcTo( const QRect &box, const QPoint &start, const QPoint &end ) override;
	void polygon16( const QRect &bounds, QList<QPoint> points ) override;
	void polyLine( const QRect &bounds, QList<QPoint> points ) override;
	void polyLine16( const QRect &bounds, QList<QPoint> points ) override;
	void polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points ) override;
	void polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points ) override;
	void polyLineTo16( const QRect &bounds, QList<QPoint> points ) override;
	void polyBezier16( const QRect &bounds, QList<QPoint> points ) override;
	void polyBezierTo16( const QRect &bounds, QList<QPoint> points ) override;
	void fillPath( const QRect &bounds ) override;
	void strokeAndFillPath( const QRect &bounds ) override;
	void strokePath( const QRect &bounds ) override;
	void setMitterLimit(quint32 limit) override;
	void setClipPath( quint32 regionMode ) override;
	void bitBlt( BitBltRecord &bitBltRecord ) override;
	void setStretchBltMode( quint32 stretchMode ) override;
	void stretchDiBits( StretchDiBitsRecord &stretchDiBitsRecord ) override;
	void alphaBlend(AlphaBlendRecord&) override;

	private:
		QDebug *m_debug{nullptr};
		QFile *m_file{nullptr};
};


}

#endif
