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

#include <QClipboard>
#include <QFileDialog>
#include <QDir>
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

LeechCraft::CommonJobAdder::CommonJobAdder (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
#if QT_VERSION >= 0x040700
	What_->setPlaceholderText (What_->toolTip ());
#endif
	QString text = XmlSettingsManager::Instance ()->
			Property ("LastWhatFolder", QString ()).toString ();
	if (!text.isEmpty ())
		What_->setText (text);
}

QString LeechCraft::CommonJobAdder::GetString () const
{
	return What_->text ();
}

void LeechCraft::CommonJobAdder::on_Browse__released ()
{
	QString name = QFileDialog::getOpenFileName (this,
			tr ("Select file"),
			XmlSettingsManager::Instance ()->Property ("LastWhatFolder",
				QDir::homePath ()).toString ());
	if (name.isEmpty ())
		return;

	What_->setText (name);
	XmlSettingsManager::Instance ()->setProperty ("LastWhatFolder", name);
}
