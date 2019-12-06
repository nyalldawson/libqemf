/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 - 2016 Ion Vasilief <ion_vasilief@yahoo.fr>

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA
*/

#include "EmfViewer.h"
#include <QApplication>

int main(int argc, char **argv)
{
#ifdef STATIC_BUILD
	Q_IMPORT_PLUGIN(qgif);
	Q_IMPORT_PLUGIN(qjpeg);
	Q_IMPORT_PLUGIN(qmng);
	Q_IMPORT_PLUGIN(qtiff);
#endif

	QApplication app(argc, argv);

	bool logMode = (argc == 3) && (QString(argv[1]) == "-v");

	EmfViewer viewer(logMode);
	viewer.setWindowIcon(QIcon(":/emfviewer.png"));
	viewer.show();

	if (argc > 1)
		viewer.loadFile(QString(argv[logMode ? 2 : 1]));

	app.installEventFilter(&viewer);
	app.exec();
}
