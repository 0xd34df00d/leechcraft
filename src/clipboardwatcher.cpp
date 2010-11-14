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

#include "clipboardwatcher.h"
#include <QApplication>
#include <QTimer>
#include <QClipboard>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	ClipboardWatcher::ClipboardWatcher (QObject *parent)
	: QObject (parent)
	{
		ClipboardWatchdog_ = new QTimer (this);
		connect (ClipboardWatchdog_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleClipboardTimer ()));
		ClipboardWatchdog_->start (2000);
	}

	ClipboardWatcher::~ClipboardWatcher ()
	{
		ClipboardWatchdog_->stop ();
		delete ClipboardWatchdog_;
	}

	void ClipboardWatcher::handleClipboardTimer ()
	{
		const QString& text = QApplication::clipboard ()->text ();
		if (text.isEmpty () || text == PreviousClipboardContents_)
			return;

		PreviousClipboardContents_ = text;

		const Entity& e = Util::MakeEntity (text.toUtf8 (),
				QString (),
				FromUserInitiated);

		if (XmlSettingsManager::Instance ()->
				property ("WatchClipboard").toBool ())
			emit gotEntity (e);
	}
};

