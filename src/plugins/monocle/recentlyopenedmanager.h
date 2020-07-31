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

namespace LC
{
namespace Monocle
{
	class RecentlyOpenedManager : public QObject
	{
		QStringList OpenedDocs_;
		QHash<QWidget*, QMenu*> Menus_;
	public:
		using PathHandler_t = std::function<void (QString)>;
	private:
		QHash<QMenu*, PathHandler_t> Handlers_;
	public:
		RecentlyOpenedManager (QObject* = nullptr);

		QMenu* CreateOpenMenu (QWidget*, const PathHandler_t&);
		void RecordOpened (const QString&);
	private:
		void UpdateMenu (QMenu*) const;
	};
}
}
