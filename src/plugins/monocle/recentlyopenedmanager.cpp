/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recentlyopenedmanager.h"
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMenu>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC
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

	QMenu* RecentlyOpenedManager::CreateOpenMenu (QWidget *docTab, const PathHandler_t& handler)
	{
		if (const auto menu = Menus_ [docTab])
			return menu;

		auto result = new QMenu (tr ("Recently opened"), docTab);
		Menus_ [docTab] = result;
		Handlers_ [result] = handler;

		UpdateMenu (result);

		connect (docTab,
				&QObject::destroyed,
				this,
				[this, docTab] { Menus_.remove (docTab); });
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

		for (const auto& menu : Menus_)
			UpdateMenu (menu);
	}

	void RecentlyOpenedManager::UpdateMenu (QMenu *menu) const
	{
		menu->clear ();

		const auto& handler = Handlers_ [menu];

		for (const auto& path : OpenedDocs_)
		{
			const QFileInfo fi { path };

			if (!fi.exists ())
			{
				qDebug () << Q_FUNC_INFO
						<< "skipping non-existent"
						<< path;
				continue;
			}

			auto act = menu->addAction (fi.fileName (),
					[handler, path] { handler (path); });
			act->setToolTip (path);
		}
	}
}
}
