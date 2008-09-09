/***************************************************************************
 *   Copyright (C) 2008 by Voker57   *
 *   voker57@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <QApplication>
#include <QTranslator>
#include <QLocale>

#include "fsirc.h"
#include "config.h"

int main(int argc, char *argv[])
{
//	This used to cause creepy errors
//	Q_INIT_RESOURCE(i18n);
     QApplication fsApp(argc, argv);
	fsApp.setApplicationName("fsirc");
	fsApp.setApplicationVersion(FS_VERSION);
	fsApp.setOrganizationName("NBL");
	QTranslator transl;
	QString localeName = QString(::getenv ("LANG")).left (2);
	if (localeName.isNull () || localeName.isEmpty ())
	localeName = QLocale::system().name();
	transl.load(QString (":/fsirc_") + localeName);
	fsApp.installTranslator (&transl);
	fsirc fsIrc;
	fsIrc.show();
	return fsApp.exec();
}
