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
#include <QString>

class QAbstractItemModel;

namespace LC::Aggregator
{
	class ItemsCategoriesTracker : public QObject
	{
		Q_OBJECT

		const QAbstractItemModel& Model_;
		QHash<QString, int> Counts_;
	public:
		explicit ItemsCategoriesTracker (QAbstractItemModel& model);
	signals:
		void categoriesChanged (const QList<QString>& categories);
	};
}
