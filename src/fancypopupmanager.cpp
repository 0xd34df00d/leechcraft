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

#include "fancypopupmanager.h"
#include <numeric>
#include <algorithm>
#include <iterator>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QSystemTrayIcon>
#include <QtDebug>
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

LeechCraft::FancyPopupManager::FancyPopupManager (QSystemTrayIcon *icon,
		QObject *parent)
: QObject (parent)
, TrayIcon_ (icon)
{
	QTimer *timer = new QTimer (this);
	connect (timer,
			SIGNAL (timeout ()),
			this,
			SLOT (timerTimeout ()));

	timer->start (500);

	connect (TrayIcon_,
			SIGNAL (messageClicked ()),
			this,
			SLOT (handleMessageClicked ()));
}

LeechCraft::FancyPopupManager::~FancyPopupManager ()
{
}

void LeechCraft::FancyPopupManager::ShowMessage (const QString& message)
{
	Popups_.push_back (message);
	Dates_ [QDateTime::currentDateTime ()] = message;

	UpdateMessage ();
}

void LeechCraft::FancyPopupManager::timerTimeout ()
{
	QDateTime current = QDateTime::currentDateTime ();

	for (dates_t::iterator i = Dates_.begin ();
			i != Dates_.end (); ++i)
		if (i->first.secsTo (current) >=
				XmlSettingsManager::Instance ()->
				property ("FinishedDownloadMessageTimeout").toInt ())
		{
			Popups_.erase (std::find (Popups_.begin (),
					Popups_.end (), i->second));
			Dates_.erase (i);

			UpdateMessage ();
			break;
		}
}

void LeechCraft::FancyPopupManager::handleMessageClicked ()
{
	Dates_.clear ();
	Popups_.clear ();
}

void LeechCraft::FancyPopupManager::UpdateMessage ()
{
	QString message;
	for (popups_t::const_iterator i = Popups_.begin (),
			begin = Popups_.begin (),
			end = Popups_.end (); i != end; ++i)
	{
		message += *i;
		message += "\r\n";
		if (std::distance (begin, i) >= 12)
			break;
	}

	if (!message.isEmpty ())
		TrayIcon_->showMessage (tr ("LeechCraft Notification"),
				message,
				QSystemTrayIcon::Information,
				XmlSettingsManager::Instance ()->
				property ("FinishedDownloadMessageTimeout").toInt () * 1000);
}

