/*
  Copyright 2008        Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 - 2010 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#ifndef EMF_RENDERER_H
#define EMF_RENDERER_H

#include <QList>
#include <QPainter>
#include <QRect> // also provides QSize, QPoint
#include <QString>
#include <QVariant>

#include "EmfEnums.h"
#include "EmfHeader.h"
#include "EmfRecords.h"
#include "EmfOutput.h"


/**
   \file

   Primary definitions for EMF output strategies
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace QEmf
{

class EmrTextObject;

/**
	QPainter based output strategy for EMF Parser.

	This class allows rendering of an EMF file to a QPixmap or any other QPaintDevice.
*/
class QEmfRenderer : public AbstractOutput
{
public:
	/**
	   Constructor.

	   This will probably need to take an enum to say what sort of output
	   we want.
	*/
	QEmfRenderer();
	QEmfRenderer( QPainter &painter, QSize &size, bool keepAspectRatio = false );
	~QEmfRenderer() override;

	void init( const Header *header ) override;
	void cleanup( const Header *header ) override;
	void eof() override;

	/**
	   The image that has been rendered to.
	*/
	QImage *image();

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
	void polyLine16( const QRect &bounds, QList<QPoint> points ) override;
	void polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points ) override;
	void polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points ) override;
	void polyLine( const QRect &bounds, QList<QPoint> points ) override;
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
	void printPainterTransform(const char *leadText);

	/// For debugging purposes: Draw the boundary box.
	void paintBounds(const Header *header);

	/// Recalculate the world transform and then apply it to the painter
	/// This must be called at the end of every function that changes the transform.
	void recalculateWorldTransform();

	/**
	   Select a stock object.

	   See [MS-EMF] Section 2.1.31.

	   \param ihObject the stock object value
	*/
	void selectStockObject( quint32 ihObject );


	/**
	   Helper routine to convert the EMF angle (centrepoint + radial endpoint) into
	   the Qt format (in degress - may need to multiply by 16 for some purposes)
	*/
	qreal angleFromArc( const QPoint &centrePoint, const QPoint &radialPoint );

	/**
	  Calculate the angular difference (span) between two angles

	  This should always be positive.
	*/
	qreal angularSpan( qreal startAngle, qreal endAngle );

	/**
	   Convert the EMF font weight scale (0..1000) to Qt equivalent.

	   This is a bit rough - the EMF spec only says 400 is normal, and
	   700 is bold.
	*/
	int convertFontWeight( quint32 emfWeight );


	Header                  *m_header{ nullptr };   // Save to be able to retain scaling.
	int                      m_painterSaves; // The number of times that save() was called.
	QSize                    m_outputSize;
	bool                     m_keepAspectRatio;

	QMap<quint32, QVariant>  m_objectTable;

	QPainterPath *m_path{ nullptr };
	bool          m_currentlyBuildingPath{false};

	QPainter                *m_painter;
	QTransform               m_worldTransform; // The transform inside the EMF.
	QTransform               m_pathTransform; // The transform inside the EMF to be applied only to the path under construction
	QTransform               m_outputTransform; // The transform that the painter already had
	qreal                    m_outputScale;
	qreal                    m_textRotation;

	// Everything that has to do with window and viewport calculation
	QPoint        m_windowOrg;
	QSize         m_windowExt;
	QPoint        m_viewportOrg;
	QSize         m_viewportExt;
	bool          m_windowExtIsSet;
	bool          m_viewportExtIsSet;
	bool          m_windowViewportIsSet;

#if 0
	// This matrix is needed because the window / viewport calculation
	// is not the last one in the chain. After that one comes the
	// transform that the painter already has when the painting
	// starts, and that one has to be saved and reapplied again after
	// the window / viewport calculation is redone.
	QTransform    m_outputTransform;
#endif

	// ----------------------------------------------------------------
	//                     The playback device context

	// The Playback Device Context (PDC) contains the following:
	//  - bitmap
	//  - brush	(part of the painter)
	//  - palette
	//  - font	(part of the painter)
	//  - pen	(part of the painter)
	//  - region
	//  - drawing mode
	//  - mapping mode
	// FIXME: what more?  textalign?  textpen?

	/**
	   The current text pen
	*/
	QPen m_textPen;

	/**
	   The current fill rule
	*/
	enum Qt::FillRule m_fillRule{Qt::OddEvenFill};

	/**
	   The current map mode
	*/
	MapMode  m_mapMode{MM_TEXT};
	/**
		The current text alignment mode
	*/
	quint32 m_textAlignMode{TA_NOUPDATECP};

	/**
	   The current coordinates
	*/
	QPoint  m_currentCoords;

	/**
		The current bitmap mask
	*/
	QBitmap  m_current_bitmap_mask;

	/**
		The current bitmap mask-in color
	*/
	QColor m_current_mask_color;
};

}

#endif
