/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDockWidget>

class QTreeView;

namespace LC::Monocle
{
	class AnnManager;
	class AnnWidget;
	struct Bookmark;
	class BookmarksStorage;
	class BookmarksWidget;
	class IDocument;
	struct LinkExecutionContext;
	class SearchTabWidget;
	class TextSearchHandler;
	class ThumbsWidget;
	class TOCWidget;
	class ViewPositionTracker;

	class Dock : public QDockWidget
	{
		Q_OBJECT

		TOCWidget& Toc_;
		BookmarksWidget& Bookmarks_;
		ThumbsWidget& Thumbnails_;
		AnnWidget& Annotations_;
		SearchTabWidget& Search_;
		QTreeView& OptionalContents_;
	public:
		struct Deps
		{
			LinkExecutionContext& LinkContext_;
			QWidget& TabWidget_;

			AnnManager& AnnotationsMgr_;
			BookmarksStorage& BookmarksStorage_;
			TextSearchHandler& SearchHandler_;
			ViewPositionTracker& ViewPosTracker_;
		};

		explicit Dock (const Deps&);

		void HandleDoc (IDocument&);
	private:
		void SetupToc (ViewPositionTracker&, LinkExecutionContext&);
		void SetupThumbnails (ViewPositionTracker&, LinkExecutionContext&);
		void SetupBookmarks ();
	signals:
		void addBookmarkRequested ();
		void removeBookmarkRequested (const Bookmark&);
		void bookmarkActivated (const Bookmark&);
	};
}
