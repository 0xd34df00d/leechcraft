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
class QTimer;

namespace LC::Aggregator
{
	class ItemActions;

	class ItemSelectionTracker : public QObject
	{
		Q_OBJECT

		QAbstractItemView& View_;

		bool TapeMode_ = false;
		QSet<IDType_t> CurrentItems_;
		QTimer& ReadMarkTimer_;
	public:
		explicit ItemSelectionTracker (QAbstractItemView&, ItemActions&, QObject* = nullptr);

		QSet<IDType_t> GetSelectedItems () const;
		void SetTapeMode (bool);
	private:
		void SaveCurrentItems (const QList<QModelIndex>&);
		void HandleCurrentRowChanged (const QModelIndex&);
		void MarkCurrentRead ();
	signals:
		void refreshItemDisplay ();
		void selectionChanged ();
	};
}
