/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>

namespace LC
{
namespace Launchy
{
	class RecentManager : public QObject
	{
		Q_OBJECT

		QStringList RecentList_;
	public:
		explicit RecentManager (QObject* = nullptr);

		bool HasRecents () const;
		bool IsRecent (const QString&) const;
		int GetRecentOrder (const QString&) const;

		void AddRecent (const QString&);
	private:
		void Save () const;
		void Load ();
	signals:
		void recentListChanged ();
	};
}
}
