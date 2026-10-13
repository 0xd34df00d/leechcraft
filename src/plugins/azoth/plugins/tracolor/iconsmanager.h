/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QIcon>
#include <interfaces/azoth/azothcommon.h>

namespace LC
{
namespace Azoth
{
namespace Tracolor
{
	class EntryEventsManager;
	class EventsSettingsManager;

	class IconsManager : public QObject
	{
		Q_OBJECT

		EntryEventsManager * const EvMgr_;
		EventsSettingsManager * const SettingsMgr_;

		struct IconsCacheEntry
		{
			QIcon Icon_;
			QColor Color_;
			double Rate_;
		};
		QHash<GlobalStrongestId, QHash<QString, IconsCacheEntry>> IconsCache_;
	public:
		IconsManager (EntryEventsManager*, EventsSettingsManager*, QObject* = nullptr);

		QList<QIcon> GetIcons (const GlobalStrongestId& entryId);
	private:
		void RegenCache (const GlobalStrongestId&);
		void RegenCaches ();
	signals:
		void iconUpdated (const GlobalStrongestId&);
	};
}
}
}
