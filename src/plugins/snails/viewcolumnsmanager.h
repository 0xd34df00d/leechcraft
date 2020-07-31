/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPair>

class QHeaderView;

namespace LC
{
namespace Snails
{
	class ViewColumnsManager : public QObject
	{
		Q_OBJECT

		QHeaderView * const View_;

		int StretchColumn_ = -1;
		QList<int> ColumnWidths_;

		bool IgnoreResizes_ = false;

		QList<QPair<int, int>> Swaps_;
	public:
		ViewColumnsManager (QHeaderView*);

		void SetStretchColumn (int);
		void SetDefaultWidth (int, int);
		void SetDefaultWidths (const QList<int>&);
		void SetDefaultWidths (const QStringList&);

		void SetSwaps (const QList<QPair<int, int>>&);

		bool eventFilter (QObject*, QEvent*);
	private slots:
		void readjustWidths ();
		void handleSectionResized (int, int, int);
		void handleSectionCountChanged (int, int);
	};
}
}
