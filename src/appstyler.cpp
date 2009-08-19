/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "appstyler.h"
#include <QStyleFactory>
#include <QApplication>
#include "xmlsettingsmanager.h"

#ifdef Q_WS_WIN
#include <QWindowsXPStyle>
#endif

namespace LeechCraft
{
	AppStyler::AppStyler (QWidget *parent)
	: QComboBox (parent)
	{
		addItems (QStyleFactory::keys ());
		reject ();
	}

	void AppStyler::accept ()
	{
		QString style = currentText ();
		XmlSettingsManager::Instance ()->
			setProperty ("AppQStyle", style);
		QApplication::setStyle (style);
	}

	void AppStyler::reject ()
	{
		QString style = XmlSettingsManager::Instance ()->
				property ("AppQStyle").toString ();
		if (style.isEmpty ())
		{
#ifdef Q_WS_WIN
			style = "Plastique";
			XmlSettingsManager::Instance ()->
				setProperty ("AppQStyle", style);
#endif
		}
		QApplication::setStyle (style);
		int index = findText (style);
		setCurrentIndex (index);
	}
};

