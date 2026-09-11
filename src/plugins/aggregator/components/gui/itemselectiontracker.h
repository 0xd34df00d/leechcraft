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
		ItemActions& Actions_;
	public:
		struct SelectedItem
		{
			IDType_t Channel_;
			IDType_t Item_;

			static SelectedItem FromIndex (const QModelIndex&);

			auto operator<=> (const SelectedItem&) const = default;
		};

		friend std::size_t qHash (const SelectedItem& item, size_t seed);
	private:
		QSet<SelectedItem> CurrentItems_;

		QTimer& ReadMarkTimer_;

		bool TapeMode_ = false;
		bool ScheduledSyncToSelection_ = false;
		bool GestureActive_ = false;
	public:
		explicit ItemSelectionTracker (QAbstractItemView&, ItemActions&, QObject* = nullptr);

		void SetTapeMode (bool);

		bool eventFilter (QObject*, QEvent*) override;
	private:
		QSet<IDType_t> GetSelectedItems () const;
		void EndGesture ();

		void HandleImmediateSelectionChange ();
		void ScheduleSyncToSelection ();
		void SyncToSelection ();
		void RearmMarkTimer ();
	signals:
		void refreshItemDisplay ();
		void selectionChanged (const QSet<IDType_t>&);
	};
}
