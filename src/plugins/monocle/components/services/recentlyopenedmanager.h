/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QHash>
#include <QStringList>

class QMenu;

namespace LC::Monocle
{
	class RecentlyOpenedManager : public QObject
	{
		QStringList OpenedDocs_;
	public:
		using PathHandler_t = std::function<void (QString)>;
	private:
		QHash<QMenu*, PathHandler_t> Handlers_;
	public:
		explicit RecentlyOpenedManager (QObject* = nullptr);

		QMenu* CreateOpenMenu (QWidget*, const PathHandler_t&);
		void RecordOpened (const QString&);
	private:
		void UpdateMenu (QMenu*, const PathHandler_t&) const;
	};
}
