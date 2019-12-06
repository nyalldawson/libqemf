/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 - 2016 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#ifndef EMFVIEWER_H
#define EMFVIEWER_H

#include <QAction>
#include <QDropEvent>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <QScrollBar>
#include <QUrl>

class EmfViewer : public QMainWindow
{
	Q_OBJECT

public:
	explicit EmfViewer(bool = false);
	void exportFile(const QString &fileName);
	void loadFile(const QString &fileName);
#ifdef Q_OS_WIN
	void loadWindowsMetafile(const QString&);
#endif

private slots:
	void slotExportFile();
	void slotOpenFile();
	void zoomIn();
	void zoomOut();
	void renderImage();
	void normalSize();
	void fitToWindow();
	void about();
	void openBackgroundDialog();
	void enableMouseWheelZoom(bool);

private:
	QPicture renderPicture();
	void exportPicture(const QString &);
	void exportPdf(const QString &);
	void exportPostScript(const QString &);
	void exportSvg(const QString &);
	bool validFileType(const QString&);
	QString emfUrl(QDropEvent*);
	bool eventFilter(QObject *, QEvent *);
	void dropEvent(QDropEvent*);
	void dragEnterEvent(QDragEnterEvent*);
	void contextMenuEvent(QContextMenuEvent *);

	void createMenus();
	void loadImage();
	void scaleImage(double);
	void setRenderHints(QPainter *);
	void showStatusBarMessage();
	void adjustScrollBar(QScrollBar *, double);
	void enableActions(bool = true);

	QAction *m_fileOpenAction;
	QAction *m_fileExportAction;
	QAction *m_fileQuitAction;
	QAction *m_zoomInAct;
	QAction *m_zoomOutAct;
	QAction *m_zoomMouseWheelAct;
	QAction *m_normalSizeAct;
	QAction *m_fitToWindowAct;
	QAction *m_aboutAct;
	QAction *m_antialiasingAct;
	QAction *m_textAntialiasingAct;
	QAction *m_backgroundColorAct;
	QAction *m_transparentBkgAct;

	QLabel *m_label;
	//! The central widget
	QScrollArea *m_scroll_area;

	//! Flag for creating a log file
	bool m_log;

	//! Keeps track of the current file path
	QString m_path;

	//! Stores the size of the current metafile image
	QSize m_size;

#ifdef Q_OS_WIN
	// Stores the data of WMF files on Windows
	QByteArray m_ba;
#endif

	//! Stores the current zoom factor
	double m_scaleFactor;

	//! Stores the current background color
	QColor m_background;

	//! Flag telling if zooming with the mouse whell is enabled
	bool m_zoom_mouse_wheel;
};
#endif
