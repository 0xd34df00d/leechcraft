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
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

IconChooser::IconChooser (const QStringList& sets, QWidget *parent)
: QComboBox (parent)
, Sets_ (sets)
{
	addItems (sets);
	reject ();
}

void IconChooser::accept ()
{
	XmlSettingsManager::Instance ()->
		setProperty ("IconSet", currentText ());
	emit requestNewIconSet ();
}

void IconChooser::reject ()
{
	int index = Sets_.indexOf (XmlSettingsManager::Instance ()->
			Property ("IconSet", "oxygen").toString ());
	setCurrentIndex (index == -1 ? 0 : index);
}

