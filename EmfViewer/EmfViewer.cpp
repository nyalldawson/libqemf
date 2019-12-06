/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2008 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 - 2016 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#include "EmfViewer.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QFileDialog>
#include <QImageWriter>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPicture>
#include <QPrinter>
#include <QStatusBar>
#include <QSvgGenerator>

#include <QEmfRenderer.h>
#include <EmfLogger.h>
#include <EmfParser.h>

#ifdef Q_OS_WIN
#include <Windows.h>
typedef struct _WmfSpecialHeader
	{
		DWORD Key;           /* Magic number (always 9AC6CDD7h) */
		WORD  Handle;        /* Metafile HANDLE number (always 0) */
		SHORT Left;          /* Left coordinate in metafile units */
		SHORT Top;           /* Top coordinate in metafile units */
		SHORT Right;         /* Right coordinate in metafile units */
		SHORT Bottom;        /* Bottom coordinate in metafile units */
		WORD  Inch;          /* Number of metafile units per inch */
		DWORD Reserved;      /* Reserved (always 0) */
		WORD  Checksum;      /* Checksum value for previous 10 WORDs */
	} WMFSPECIAL;
#endif

#ifdef Q_OS_MAC
	#include <CoreFoundation/CoreFoundation.h>
#endif

EmfViewer::EmfViewer(bool log)
	: QMainWindow()
{
	m_log = log;
	m_path = "";
	m_background = QColor(Qt::white);
	m_zoom_mouse_wheel = true;

	setWindowTitle(tr("EmfViewer"));
	statusBar()->setSizeGripEnabled(true);

	m_label = new QLabel;
	m_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	m_scroll_area = new QScrollArea;
	m_scroll_area->setWidget(m_label);
	m_scroll_area->setBackgroundRole(QPalette::Dark);
	setCentralWidget(m_scroll_area);

	setAcceptDrops(true);

	createMenus();
	resize(500, 400);
}

void EmfViewer::createMenus()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileOpenAction = fileMenu->addAction(tr("&Open..."), this, SLOT(slotOpenFile()), tr("Ctrl+O"));
	m_fileExportAction = fileMenu->addAction(tr("&Save As..."), this, SLOT(slotExportFile()), tr("Ctrl+S"));
	fileMenu->addSeparator();
	m_fileQuitAction = fileMenu->addAction(tr("&Quit"), this, SLOT(close()), tr("Ctrl+Q"));

	QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
	m_zoomInAct = viewMenu->addAction(tr("Zoom &In"), this, SLOT(zoomIn()), tr("Ctrl++"));
	m_zoomOutAct = viewMenu->addAction(tr("Zoom &Out"), this, SLOT(zoomOut()), tr("Ctrl+-"));
	viewMenu->addSeparator();
	m_normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, SLOT(normalSize()), tr("Ctrl+N"));
	m_fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, SLOT(fitToWindow()), tr("Ctrl+F"));
	viewMenu->addSeparator();
	m_zoomMouseWheelAct = viewMenu->addAction(tr("Enable Mouse &Wheel Zoom"));
	if (m_zoomMouseWheelAct){
		m_zoomMouseWheelAct->setShortcut(QKeySequence(tr("Ctrl+Alt+W")));
		m_zoomMouseWheelAct->setCheckable(true);
		m_zoomMouseWheelAct->setChecked(true);
		connect(m_zoomMouseWheelAct, SIGNAL(toggled(bool)), this, SLOT(enableMouseWheelZoom(bool)));
	}

	QMenu *displayMenu = menuBar()->addMenu(tr("&Render"));
	m_antialiasingAct = displayMenu->addAction(tr("Antia&liasing"), this, SLOT(renderImage()), tr("Ctrl+L"));
	m_antialiasingAct->setCheckable(true);
	m_antialiasingAct->setChecked(true);

	m_textAntialiasingAct = displayMenu->addAction(tr("T&ext antialiasing"), this, SLOT(renderImage()), tr("Ctrl+E"));
	m_textAntialiasingAct->setCheckable(true);
	m_textAntialiasingAct->setChecked(true);

	QMenu *backgroundMenu = menuBar()->addMenu(tr("&Background"));
	m_backgroundColorAct = backgroundMenu->addAction("&" + tr("Color") + "...", this, SLOT(openBackgroundDialog()), tr("Ctrl+C"));
	m_transparentBkgAct = backgroundMenu->addAction("&" + tr("Transparent"), this, SLOT(renderImage()), tr("Ctrl+T"));
	m_transparentBkgAct->setCheckable(true);
	m_transparentBkgAct->setChecked(false);

	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(tr("&About"), this, SLOT(about()), tr("Ctrl+A"));

	enableActions(false);
}

#ifdef Q_OS_WIN
//First try to convert the Windows metafile to an enhanced metafile and then parse the result using QEmfRenderer
void EmfViewer::loadWindowsMetafile(const QString& fileName)
{
	m_size = m_scroll_area->size();

	HENHMETAFILE hemf = NULL;
	FILE* fp = fopen(fileName.toStdString().c_str(), "rb");
	fseek(fp, 0, SEEK_END);

	fpos_t n64FSize;
	fgetpos(fp, &n64FSize);
	int nFSize = (int)n64FSize;
	fseek(fp, 0, SEEK_SET);

	DWORD dwKey;
	fread(&dwKey, 4, 1, fp);
	fseek(fp, 0, SEEK_SET);
	if (dwKey == 0x9AC6CDD7){//it is a replaceable metafile
		WMFSPECIAL wmfsHdr;
		fread(&wmfsHdr, 22, 1, fp);
		nFSize -= 22;
		m_size = QSize(m_label->logicalDpiX()*(wmfsHdr.Right - wmfsHdr.Left)/wmfsHdr.Inch,
					   m_label->logicalDpiY()*(wmfsHdr.Bottom - wmfsHdr.Top)/wmfsHdr.Inch);
	}

	unsigned char *pBuf = new unsigned char[nFSize];
	fread(pBuf, 1, nFSize, fp);
	hemf = SetWinMetaFileBits(nFSize, pBuf, NULL, NULL);
	delete[] pBuf;
	fclose(fp);

	if (hemf){
		unsigned int emfSize = GetEnhMetaFileBits(hemf, 0, NULL);
		if (!emfSize){
			DeleteEnhMetaFile(hemf);
			return;
		}

		char *pBuf = new char[emfSize];
		emfSize = GetEnhMetaFileBits(hemf, emfSize, (LPBYTE)pBuf);
		if (!emfSize){
			DeleteEnhMetaFile(hemf);
			delete[] pBuf;
			return;
		}

		m_ba = QByteArray(pBuf, emfSize);
		DeleteEnhMetaFile(hemf);
		delete[] pBuf;

		QEmf::EmfParser parser;
		parser.setParseHeaderOnly();
		if (!parser.load(m_ba))
			return;

		loadImage();
	}
}
#endif

void EmfViewer::loadFile(const QString &fileName)
{
#ifdef Q_OS_WIN
	if (!m_ba.isEmpty())
		m_ba.clear();
#endif

	m_path = fileName;

	QEmf::EmfParser parser;
	parser.setParseHeaderOnly();
	if (!parser.load(fileName)){
#ifdef Q_OS_WIN
		if (fileName.endsWith(".wmf"))
			loadWindowsMetafile(fileName);
#endif
		return;
	}

	m_size = parser.bounds().size();
	loadImage();
}

void EmfViewer::loadImage()
{
	if (m_log){
		QEmf::EmfLogger logger(QFileInfo(m_path).baseName() + "_log.txt");
		logger.load(m_path);
	}

	m_scaleFactor = 1.0;
	renderImage();

	int w = width(), h = height();
	QRect frameRect = frameGeometry();
	int dw = abs(frameRect.width() - w), dh = abs(frameRect.height() - h);
	if ((w < m_size.width()) || (h < m_size.height())){
		QSize maxSize = QSize(1000, 800);
		QSize newSize = (m_size + QSize(dw, statusBar()->height() + dh)).boundedTo(maxSize);
		resize(newSize);
		if ((newSize.width() == maxSize.width()) || (newSize.height() == maxSize.height()))
			fitToWindow();
	}

	enableActions(true);
	setWindowTitle(tr("EmfViewer") + " - " + m_path);
}

void EmfViewer::setRenderHints(QPainter *p)
{
	if (!p)
		return;

	p->setRenderHint(QPainter::Antialiasing, m_antialiasingAct->isChecked());
	p->setRenderHint(QPainter::TextAntialiasing, m_textAntialiasingAct->isChecked());
	p->setRenderHints(QPainter::SmoothPixmapTransform | QPainter::NonCosmeticDefaultPen);
}

void EmfViewer::openBackgroundDialog()
{
	QColor color = QColorDialog::getColor(m_background, this);
	if (color.isValid()){
		m_background = color;
		m_transparentBkgAct->setChecked(false);
		renderImage();
	}
}

void EmfViewer::slotOpenFile()
{
	QString filter = "Enhanced Metafile (*.emf)";
#ifdef Q_OS_WIN
	filter = "Metafiles (*emf; *.wmf)";
#endif
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Metafile"), m_path, filter);
	if (!fileName.isEmpty())
		loadFile(fileName);
}

void EmfViewer::slotExportFile()
{
	QStringList filters;
	foreach (QByteArray ba, QImageWriter::supportedImageFormats())
		filters.append("*." + QString(ba).toLower());
	filters.append("*.eps");
	filters.append("*.pic");
	filters.append("*.pdf");
	filters.append("*.svg");
	filters.sort();

	QString filter = filters.join(";;");
	QString path = m_path, selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export Metafile As"), path.remove(".emf").remove(".wmf"), filter, &selectedFilter);
	if (!fileName.isEmpty()){
	#ifdef Q_OS_LINUX
		if (!fileName.endsWith(selectedFilter))
			fileName.append(selectedFilter.remove("*"));
	#endif
		exportFile(fileName);
	}
}

QPicture EmfViewer::renderPicture()
{
	QPicture picture;
	QPainter painter;
	painter.begin(&picture);// paint in picture

	setRenderHints(&painter);

	QSize size = m_label->size();

	if (!m_transparentBkgAct->isChecked())
		painter.fillRect(QRect(QPoint(0, 0), size), QBrush(m_background));

	QEmf::QEmfRenderer renderer(painter, size);
#ifdef Q_OS_WIN
	if (!m_ba.isEmpty())
		renderer.load(m_ba);
	else
		renderer.load(m_path);
#else
	renderer.load(m_path);
#endif

	painter.end();// painting done
	return picture;
}

void EmfViewer::exportPicture(const QString &fileName)
{
	renderPicture().save(fileName);
}

void EmfViewer::exportPdf(const QString &fileName)
{
	QPrinter printer;
	printer.setFontEmbeddingEnabled(true);
	printer.setCreator("EmfViewer");
	printer.setFullPage(true);
	printer.setColorMode(QPrinter::Color);
	printer.setPaperSize(m_label->size(), QPrinter::DevicePixel);
	printer.setResolution(logicalDpiX());//we set screen resolution as default
	printer.setOutputFileName(fileName);

	QPainter paint(&printer);
	renderPicture().play(&paint);
	paint.end();
}

void EmfViewer::exportPostScript(const QString &fileName)
{
#if QT_VERSION < 0x050000
	QPrinter printer;
	printer.setOutputFormat(QPrinter::PostScriptFormat);
	printer.setFontEmbeddingEnabled(true);
	printer.setCreator("EmfViewer");
	printer.setFullPage(true);
	printer.setColorMode(QPrinter::Color);
	printer.setPaperSize(m_label->size(), QPrinter::DevicePixel);
	printer.setResolution(logicalDpiX());//we set screen resolution as default
	printer.setOutputFileName(fileName);

	QPainter paint(&printer);
	renderPicture().play(&paint);
	paint.end();
#else
	QMessageBox::critical(this, tr("Export error"),
	tr("When built using <a href=\"https://www.qt.io\">Qt 5</a> <b>EmfViewer</b> must use the <a href=\"http://soft.proindependent.com/eps\">EpsEngine</a> library in order to export to (Encapsulated) PostScript format."));
#endif
}


void EmfViewer::exportSvg(const QString &fileName)
{
	QSize size = m_label->size();

	QSvgGenerator svg;
	svg.setFileName(fileName);
	svg.setDescription("Generated with EmfViewer");
	svg.setSize(size);
	svg.setViewBox(QRect(0, 0, size.width(), size.height()));
	svg.setResolution(logicalDpiX());

	QPainter paint(&svg);
	renderPicture().play(&paint);
	paint.end();
}

void EmfViewer::exportFile(const QString &fileName)
{
	if (fileName.endsWith(".pic")){
		exportPicture(fileName);
		return;
	} else if (fileName.endsWith(".pdf")){
		exportPdf(fileName);
		return;
	} else if (fileName.endsWith(".eps")){
		exportPostScript(fileName);
		return;
	} else if (fileName.endsWith(".svg")){
		exportSvg(fileName);
		return;
	}

	QSize size = m_label->size();

	QPixmap pix = QPixmap(size);
	QColor color = (m_transparentBkgAct->isChecked() && (fileName.endsWith(".png") || fileName.endsWith(".tif") || fileName.endsWith(".tiff"))) ?
					QColor(Qt::transparent) : m_background;
	pix.fill(color);

	QPainter painter(&pix);
	setRenderHints(&painter);

	QEmf::QEmfRenderer renderer(painter, size);
#ifdef Q_OS_WIN
	if (!m_ba.isEmpty())
		renderer.load(m_ba);
	else
		renderer.load(m_path);
#else
	renderer.load(m_path);
#endif

	painter.end();
	pix.save(fileName);
}

void EmfViewer::enableMouseWheelZoom(bool on)
{
	m_zoom_mouse_wheel = on;
}

void EmfViewer::zoomIn()
{
	scaleImage(1.25);
}

void EmfViewer::zoomOut()
{
	scaleImage(0.8);
}

void EmfViewer::normalSize()
{
	m_scaleFactor = 1.0;
	scaleImage(m_scaleFactor);
}

void EmfViewer::fitToWindow()
{
	double scaleX = double(width() - 10)/double(m_size.width());
	double scaleY = double(height() - statusBar()->height())/double(m_size.height());

	normalSize();
	scaleImage(qMin(scaleX, scaleY));
}

void EmfViewer::renderImage()
{
	QSize size = m_scaleFactor*m_size;

	QPixmap pix(size);
	pix.fill(m_transparentBkgAct->isChecked() ? QColor(Qt::transparent) : m_background);

	QPainter painter(&pix);
	setRenderHints(&painter);

	QEmf::QEmfRenderer renderer(painter, size, true);
#ifdef Q_OS_WIN
	if (!m_ba.isEmpty())
		renderer.load(m_ba);
	else
		renderer.load(m_path);
#else
	renderer.load(m_path);
#endif

	m_label->setPixmap(pix);
	m_label->resize(size);

	showStatusBarMessage();
}

void EmfViewer::scaleImage(double factor)
{
	m_scaleFactor *= factor;

	renderImage();

	adjustScrollBar(m_scroll_area->horizontalScrollBar(), factor);
	adjustScrollBar(m_scroll_area->verticalScrollBar(), factor);
}

void EmfViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
	scrollBar->setValue(int(factor*scrollBar->value() + ((factor - 1)*scrollBar->pageStep()/2)));
}

void EmfViewer::showStatusBarMessage()
{
	QString msg = tr("Image size") + ": "+ QString::number(m_size.width()) + "x" + QString::number(m_size.height()) + " " + tr("pixels");
	msg.append(" - " + tr("Zoom") + ": "+ QString::number(100*m_scaleFactor, 'f', 1) + "% - ");
	msg.append(tr("Antia&liasing").remove("&") + ": "+ (m_antialiasingAct->isChecked() ? tr("on") : tr("off")) + " - ");
	msg.append(tr("T&ext antialiasing").remove("&") + ": "+ (m_textAntialiasingAct->isChecked() ? tr("on") : tr("off")));

	statusBar()->showMessage(msg);
}

void EmfViewer::enableActions(bool on)
{
	m_fileExportAction->setEnabled(on);
	m_zoomInAct->setEnabled(on);
	m_zoomOutAct->setEnabled(on);
	m_zoomMouseWheelAct->setEnabled(on);
	m_normalSizeAct->setEnabled(on);
	m_fitToWindowAct->setEnabled(on);
	m_antialiasingAct->setEnabled(on);
	m_textAntialiasingAct->setEnabled(on);
	m_backgroundColorAct->setEnabled(on);
	m_transparentBkgAct->setEnabled(on);
}

void EmfViewer::about()
{
	QMessageBox::about(this, tr("About EmfViewer"),
	tr("<b>EmfViewer</b> uses the <b>QEmfRenderer</b> class provided by <a href=\"http://soft.proindependent.com/qemf\">libqemf</a> \
	in order to display the contents of Windows Enhanced Metafiles (*.emf) and to convert them to the raster image formats supported by the \
	<a href=\"https://www.qt.io\">Qt framework</a>.<br><br>Built using Qt version %1.").arg(qVersion()));
}

void EmfViewer::dragEnterEvent(QDragEnterEvent* e)
{
	if (emfUrl(e).isEmpty())
		e->ignore();
	else
		e->accept();
}

bool EmfViewer::validFileType(const QString& fn)
{
	if (fn.isEmpty())
		return false;

	QString ext = QFileInfo(fn).suffix().toLower();
  return (ext == "emf"
#ifdef Q_OS_WIN
	|| ext == "wmf"
#endif
  );
}

QString EmfViewer::emfUrl(QDropEvent* e)
{
	if (!e->mimeData()->hasUrls())
		return "";

	QList<QUrl> urls = e->mimeData()->urls();
	QStringList fileNames;
	foreach (QUrl url, urls){
		QString path = url.toLocalFile();
	#ifdef Q_OS_MAC
		#if QT_VERSION < 0x050000
		if ((QSysInfo::MacintoshVersion >= QSysInfo::MV_10_10)){
			CFStringRef relCFStringRef = CFStringCreateWithCString(NULL, path.toUtf8().constData(), kCFStringEncodingUTF8);
			CFURLRef relCFURL = CFURLCreateWithFileSystemPath(NULL, relCFStringRef, kCFURLPOSIXPathStyle,false);
			CFErrorRef error = 0;
			CFURLRef absCFURL = CFURLCreateFilePathURL(NULL, relCFURL, &error);
			if (!error){
				static const CFIndex maxAbsPathCStrBufLen = 4096;
				char absPathCStr[maxAbsPathCStrBufLen];
				if (CFURLGetFileSystemRepresentation(absCFURL, true, reinterpret_cast<UInt8 *>(&absPathCStr[0]), maxAbsPathCStrBufLen))
					path = QString(absPathCStr);
			}
			CFRelease(absCFURL);
			CFRelease(relCFURL);
			CFRelease(relCFStringRef);
		}
		#endif
	#endif
		fileNames << path;
	}

	if (!fileNames.isEmpty()){
		QString fn = fileNames.first();
		if (validFileType(fn))
			return fn;
	}

	return "";
}

void EmfViewer::dropEvent(QDropEvent* e)
{
	if (!isActiveWindow())
		raise();

	QString fn = emfUrl(e);
	if (!fn.isEmpty()){
		loadFile(fn);
		e->acceptProposedAction();
		e->accept();
	} else
		e->ignore();
}

bool EmfViewer::eventFilter(QObject *, QEvent *event)
{
	switch (event->type()){
		case QEvent::FileOpen:
		{
			QString file = static_cast<QFileOpenEvent *>(event)->file();
			if (validFileType(file))
				loadFile(file);
			return true;
		}

		case QEvent::Wheel:
		{
			if (!m_zoom_mouse_wheel)
				return false;

			QWheelEvent *we = static_cast<QWheelEvent *>(event);
			if (!we || (we->modifiers() != Qt::ControlModifier))
				return false;

			double degrees = (double)we->delta()/8.0;
			double scaleFactor = 1 + degrees/360.0;
			double zoom = 100*m_scaleFactor*scaleFactor;
			if ((zoom < 3) || (zoom > 1e4))
				return false;

			scaleImage(scaleFactor);
			return true;
		}

		default:
			return false;
	}
}

void EmfViewer::contextMenuEvent(QContextMenuEvent *e)
{
	QMenu *menu = new QMenu;
	menu->addAction(m_fileOpenAction);
	menu->addAction(m_fileExportAction);

	menu->addSeparator();

	menu->addAction(m_zoomInAct);
	menu->addAction(m_zoomOutAct);
	menu->addAction(m_zoomMouseWheelAct);
	menu->addSeparator();
	menu->addAction(m_normalSizeAct);
	menu->addAction(m_fitToWindowAct);

	menu->addSeparator();

	menu->addAction(m_antialiasingAct);
	menu->addAction(m_textAntialiasingAct);

	menu->addSeparator();

	QMenu *backgroundMenu = menu->addMenu(tr("&Background"));
	backgroundMenu->addAction(m_backgroundColorAct);
	backgroundMenu->addAction(m_transparentBkgAct);

	menu->addSeparator();
	menu->addAction(m_fileQuitAction);

	menu->popup(mapToGlobal(e->pos()));

	e->accept();
}
