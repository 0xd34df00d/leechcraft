/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "components/actions/itemactions.h"

class QAbstractItemView;

namespace LC::Aggregator
{
	class ItemActions;

	class ItemSelectionTracker : public QObject
	{
		Q_OBJECT

		bool EmitRefreshes_ = true;
	public:
		explicit ItemSelectionTracker (QAbstractItemView&, ItemActions&, QObject* = nullptr);

		void SetItemDependsOnSelection (bool);
	signals:
		void refreshItemDisplay ();
	};
}
