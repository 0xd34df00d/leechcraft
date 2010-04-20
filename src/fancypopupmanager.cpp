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

void LeechCraft::FancyPopupManager::ShowMessage (const LeechCraft::DownloadEntity& e)
{
	Popups_.push_back (e);
	Dates_ [e] = QDateTime::currentDateTime ();

	UpdateMessage ();
}

void LeechCraft::FancyPopupManager::timerTimeout ()
{
	QDateTime current = QDateTime::currentDateTime ();

	Q_FOREACH (DownloadEntity e, Popups_)
	{
		if (Dates_ [e].secsTo (current) >=
				XmlSettingsManager::Instance ()->
				property ("FinishedDownloadMessageTimeout").toInt ())
		{
			Popups_.removeAll (e);
			Dates_.remove (e);
		}
	}
}

void LeechCraft::FancyPopupManager::handleMessageClicked ()
{
	Dates_.clear ();
	Popups_.clear ();
}

void LeechCraft::FancyPopupManager::UpdateMessage ()
{
	if (Popups_.isEmpty ())
		return;

	Priority maxPriority = PLog_;

	QString message;
	for (popups_t::const_iterator i = Popups_.begin (),
			begin = Popups_.begin (),
			end = Popups_.end (); i != end; ++i)
	{
		message += i->Additional_ ["Text"].toString ();
		message += "\r\n";

		Priority prio = static_cast<Priority> (i->Additional_ ["Priority"].toInt ());
		if (prio > maxPriority)
			maxPriority = prio;
	}

	QSystemTrayIcon::MessageIcon mi = QSystemTrayIcon::Information;
	switch (maxPriority)
	{
		case PWarning_:
			mi = QSystemTrayIcon::Warning;
			break;
		case PCritical_:
			mi = QSystemTrayIcon::Critical;
		default:
			break;
	}

	TrayIcon_->showMessage (tr ("LeechCraft Notification"),
			message,
			mi,
			XmlSettingsManager::Instance ()->
				property ("FinishedDownloadMessageTimeout").toInt () * 1000);
}

