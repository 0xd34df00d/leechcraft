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
	class DocumentTab;

	class AnnManager;
	class AnnWidget;
	class BookmarksWidget;
	class DocumentBookmarksManager;
	class SearchTabWidget;
	class TextSearchHandler;
	class ThumbsWidget;
	class TOCWidget;
	class ViewPositionTracker;

	class Dock : public QDockWidget
	{
		TOCWidget& Toc_;
		BookmarksWidget& Bookmarks_;
		ThumbsWidget& Thumbnails_;
		AnnWidget& Annotations_;
		SearchTabWidget& Search_;
		QTreeView& OptionalContents_;
	public:
		struct Deps
		{
			DocumentTab& DocTab_;

			AnnManager& AnnotationsMgr_;
			DocumentBookmarksManager& BookmarksMgr_;
			TextSearchHandler& SearchHandler_;
			ViewPositionTracker& ViewPosTracker_;
		};

		explicit Dock (const Deps&);
	private:
		void SetupToc (ViewPositionTracker&, DocumentTab&);
		void SetupThumbnails (ViewPositionTracker&, DocumentTab&);
	};
}
