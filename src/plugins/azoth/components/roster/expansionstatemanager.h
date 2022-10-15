/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <QObject>

class QModelIndex;
class QPersistentModelIndex;
class QTreeView;

namespace LC::Azoth
{
	class SortFilterProxyModel;

	class ExpansionStateManager : public QObject
	{
		SortFilterProxyModel& Model_;
		QTreeView& View_;

		QHash<QString, bool> FstLevelExpands_;
		QHash<QString, QHash<QString, bool>> SndLevelExpands_;
	public:
		ExpansionStateManager (SortFilterProxyModel&, QTreeView&, QObject* = nullptr);

		void SetMucMode (bool);
	private:
		void SaveWholeModeExpansions ();
		void RestoreWholeModeExpansions ();

		void ReexpandTree ();
		void HandleRowsInserted (const QModelIndex&, int, int);

		void ExpandLater (const QModelIndex&);
		void ExpandIndex (const QPersistentModelIndex&);
	};
}
