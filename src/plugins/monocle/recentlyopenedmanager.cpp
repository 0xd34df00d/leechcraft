/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "recentlyopenedmanager.h"
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
namespace Monocle
{
	RecentlyOpenedManager::RecentlyOpenedManager (QObject *parent)
	: QObject (parent)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		OpenedDocs_ = settings.value ("RecentlyOpened").toStringList ();
	}

	QMenu* RecentlyOpenedManager::CreateOpenMenu (QWidget *docTab)
	{
		if (Menus_.contains (docTab))
			return Menus_ [docTab];

		auto result = new QMenu (tr ("Recently opened"), docTab);
		UpdateMenu (result);
		Menus_ [docTab] = result;
		connect (docTab,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleDocTabDestroyed ()));
		return result;
	}

	void RecentlyOpenedManager::RecordOpened (const QString& path)
	{
		if (OpenedDocs_.value (0) == path)
			return;

		if (OpenedDocs_.contains (path))
			OpenedDocs_.removeAll (path);
		OpenedDocs_.prepend (path);

		const int listLength = XmlSettingsManager::Instance ()
				.property ("RecentlyOpenedListSize").toInt ();
		if (OpenedDocs_.size () > listLength)
			OpenedDocs_.erase (OpenedDocs_.begin () + listLength, OpenedDocs_.end ());

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		settings.setValue ("RecentlyOpened", OpenedDocs_);

		for (const auto& menu : Menus_.values ())
			UpdateMenu (menu);
	}

	void RecentlyOpenedManager::UpdateMenu (QMenu *menu)
	{
		menu->clear ();
		for (const auto& path : OpenedDocs_)
		{
			auto act = menu->addAction (QFileInfo (path).fileName ());
			act->setProperty ("Path", path);
			act->setToolTip (path);
		}
	}

	void RecentlyOpenedManager::handleDocTabDestroyed ()
	{
		Menus_.remove (static_cast<QWidget*> (sender ()));
	}

	void RecentlyOpenedManager::handleActionTriggered ()
	{
		const auto& path = sender ()->property ("Path").toString ();
		const QFileInfo fi (path);
		if (!fi.exists ())
		{
			QMessageBox::warning (0,
					"LeechCraft",
					tr ("Seems like file %1 doesn't exist anymore.")
						.arg ("<em>" + fi.fileName () + "</em>"));
			return;
		}
	}
}
}
