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
 * along with this program.  If kn, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "fancypopupmanager.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "kineticnotification.h"
#include "skinengine.h"

using namespace LeechCraft;

LeechCraft::FancyPopupManager::FancyPopupManager (QObject *parent)
: QObject (parent)
{
}

LeechCraft::FancyPopupManager::~FancyPopupManager ()
{
}

void LeechCraft::FancyPopupManager::ShowMessage (const LeechCraft::Notification& p)
{
	KineticNotification *kn = new KineticNotification (QString::number (rand ()),
			XmlSettingsManager::Instance ()->
				property ("FinishedDownloadMessageTimeout").toInt () * 1000);
	
	QString mi = "information";
	switch (p.Priority_)
	{
		case Notification::PWarning_:
			mi = "warning";
			break;
		case Notification::PCritical_:
			mi = "error";
		default:
			break;
	}

	QMap<int, QString> sizes = SkinEngine::Instance ().GetIconPath (mi);
	int size = 0;
	if (!sizes.contains (size))
		size = sizes.keys ().last ();
	QString path = sizes [size];
	kn->setMessage (p.Header_, p.Text_, path);
	kn->send ();
}

