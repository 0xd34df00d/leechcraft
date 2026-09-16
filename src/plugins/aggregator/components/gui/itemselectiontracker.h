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
#include <QTimer>
#include <util/gui/viewselectiontracker.h>
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

		Util::ViewSelectionTracker Tracker_;
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

		QTimer ReadMarkTimer_;
	public:
		explicit ItemSelectionTracker (QAbstractItemView&, ItemActions&, QObject* = nullptr);
	private:
		QSet<IDType_t> GetSelectedItems () const;

		void HandleSelectionChanged (const QList<QModelIndex>&);
		void RearmMarkTimer ();
	signals:
		void refreshItemDisplay ();
		void selectionChanged (const QSet<IDType_t>&);
	};
}
