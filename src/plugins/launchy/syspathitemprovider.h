/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QStandardItem;
class QStandardItemModel;

namespace LC
{
namespace Launchy
{
	class SysPathItemProvider : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;

		bool SearchPathScheduled_ = false;
		QString CurrentQuery_;

		QStandardItem *PathItem_;
	public:
		explicit SysPathItemProvider (QStandardItemModel*, QObject* = nullptr);

		void HandleQuery (const QString&);
	private:
		void ScheduleSearch ();
	private slots:
		void searchPath ();
	};
}
}
