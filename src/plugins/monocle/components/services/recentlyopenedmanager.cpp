/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recentlyopenedmanager.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QMenu>
#include <QMimeDatabase>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "xmlsettingsmanager.h"

namespace LC::Monocle
{
	RecentlyOpenedManager::RecentlyOpenedManager (QObject *parent)
	: QObject { parent }
	{
		OpenedDocs_ = XmlSettingsManager::Instance ().property ("RecentlyOpened").toStringList ();
	}

	QMenu* RecentlyOpenedManager::CreateOpenMenu (QWidget *parent, const PathHandler_t& handler)
	{
		auto menu = new QMenu (tr ("Recently opened"), parent);
		Handlers_ [menu] = handler;
		UpdateMenu (menu, handler);
		connect (menu,
				&QObject::destroyed,
				this,
				[this, menu] { Handlers_.remove (menu); });
		return menu;
	}

	void RecentlyOpenedManager::RecordOpened (const QString& path)
	{
		if (OpenedDocs_.value (0) == path)
			return;

		if (OpenedDocs_.contains (path))
			OpenedDocs_.removeAll (path);
		OpenedDocs_.prepend (path);

		const int listLength = XmlSettingsManager::Instance ().property ("RecentlyOpenedListSize").toInt ();
		if (OpenedDocs_.size () > listLength)
			OpenedDocs_.erase (OpenedDocs_.begin () + listLength, OpenedDocs_.end ());

		XmlSettingsManager::Instance ().setProperty ("RecentlyOpened", OpenedDocs_);

		for (const auto& [menu, handler] : Util::Stlize (Handlers_))
			UpdateMenu (menu, handler);
	}

	void RecentlyOpenedManager::UpdateMenu (QMenu *menu, const PathHandler_t& handler) const
	{
		menu->clear ();

		const QMimeDatabase mimeDb;
		for (const auto& path : OpenedDocs_)
		{
			const QFileInfo fi { path };
			if (!fi.exists ())
				continue;

			auto act = menu->addAction (fi.fileName (), [handler, path] { handler (path); });
			act->setIcon (QIcon::fromTheme (mimeDb.mimeTypeForName (path).iconName ()));
			act->setToolTip (path);
		}
	}
}
