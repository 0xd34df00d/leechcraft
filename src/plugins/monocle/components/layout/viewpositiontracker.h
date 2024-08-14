/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC::Monocle
{
	class PagesLayoutManager;
	class PagesView;
	struct PageRelativeRect;

	class ViewPositionTracker : public QObject
	{
		Q_OBJECT

		PagesLayoutManager& LayoutManager_;
		PagesView& View_;

		int PrevCurrentPage_ = -1;

		bool UpdatesEnabled_ = true;
	public:
		explicit ViewPositionTracker (PagesView&, PagesLayoutManager&, QObject* = nullptr);

		void SetUpdatesEnabled (bool);
	private:
		void Update ();
		void RegenPageVisibility ();
	signals:
		void currentPageChanged (int);
		void pagesVisibilityChanged (const QMap<int, PageRelativeRect>&);
	};
}
