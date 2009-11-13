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

#include "iconchooser.h"
#include <QtDebug>
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

IconChooser::IconChooser (const QStringList& sets, QWidget *parent)
: QComboBox (parent)
, Sets_ (sets)
{
#ifndef Q_NO_DEBUG
	qDebug () << Q_FUNC_INFO
		<< sets;
#endif
	addItems (sets);
	reject ();
}

void IconChooser::accept ()
{
	if (XmlSettingsManager::Instance ()->
			property ("IconSet").toString () == currentText ())
		return;

	XmlSettingsManager::Instance ()->
		setProperty ("IconSet", currentText ());
	emit requestNewIconSet ();
}

void IconChooser::reject ()
{
	QString iconset = XmlSettingsManager::Instance ()->
			property ("IconSet").toString ();
#ifndef Q_NO_DEBUG
	qDebug () << Sets_
		<< iconset;
#endif
	if (iconset.isEmpty ())
	{
		iconset = "oxygen";
		XmlSettingsManager::Instance ()->
			setProperty ("IconSet", iconset);
		emit requestNewIconSet ();
	}
	int index = Sets_.indexOf (iconset);
	setCurrentIndex (index == -1 ? 0 : index);
}

