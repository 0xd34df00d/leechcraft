/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include "components/actions/itemactions.h"
#include "common.h"

class QAbstractItemView;
class QModelIndex;

namespace LC::Aggregator
{
	class ItemActions;

	class ItemSelectionTracker : public QObject
	{
		Q_OBJECT

		bool EmitRefreshes_ = true;
		QSet<IDType_t> CurrentItems_;
	public:
		explicit ItemSelectionTracker (QAbstractItemView&, ItemActions&, QObject* = nullptr);

		void SetItemDependsOnSelection (bool);
	private:
		void SaveCurrentItems (const QList<QModelIndex>&);
	signals:
		void refreshItemDisplay ();
	};
}
