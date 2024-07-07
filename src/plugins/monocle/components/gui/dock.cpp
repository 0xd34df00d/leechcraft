/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dock.h"
#include <QTabWidget>
#include <QTreeView>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/imwproxy.h>
#include <util/sll/qtutil.h>
#include "interfaces/monocle/ihaveoptionalcontent.h"
#include "components/layout/viewpositiontracker.h"
#include "annmanager.h"
#include "annwidget.h"
#include "bookmarkswidget.h"
#include "linkexecutioncontext.h"
#include "pagegraphicsitem.h"
#include "searchtabwidget.h"
#include "thumbswidget.h"
#include "tocwidget.h"
#include "xmlsettingsmanager.h"

namespace LC::Monocle
{
	namespace
	{
		auto GetLastDockWidgetArea ()
		{
			auto dwa = XmlSettingsManager::Instance ().Property ("DockWidgetArea", Qt::RightDockWidgetArea).toInt ();
			if (dwa != Qt::NoDockWidgetArea)
				return static_cast<Qt::DockWidgetArea> (dwa);

			return Qt::RightDockWidgetArea;
		}
	}

	Dock::Dock (const Deps& deps)
	: QDockWidget { tr ("Monocle dock") }
	, Toc_ { *new TOCWidget }
	, Bookmarks_ { *new BookmarksWidget { deps.BookmarksStorage_ } }
	, Thumbnails_ { *new ThumbsWidget {} }
	, Annotations_ { *new AnnWidget { deps.AnnotationsMgr_ } }
	, Search_ { *new SearchTabWidget { deps.SearchHandler_ } }
	, OptionalContents_ { *new QTreeView {} }
	{
		connect (this,
				&QDockWidget::dockLocationChanged,
				[] (Qt::DockWidgetArea area)
				{
					if (area != Qt::AllDockWidgetAreas && area != Qt::NoDockWidgetArea)
						XmlSettingsManager::Instance ().setProperty ("DockWidgetArea", area);
				});
		connect (this,
				&QDockWidget::visibilityChanged,
				[] (bool visible) { XmlSettingsManager::Instance ().setProperty ("DockWidgetVisible", visible); });

		setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
		setAllowedAreas (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		toggleViewAction ()->setProperty ("ActionIcon", "view-sidetree");

		const auto mgr = GetProxyHolder ()->GetIconThemeManager ();
		auto tabs = new QTabWidget { this };
		tabs->setTabPosition (QTabWidget::West);
		tabs->addTab (&Toc_, mgr->GetIcon ("view-table-of-contents-ltr"_qs), tr ("Table of contents"));
		tabs->addTab (&Bookmarks_, mgr->GetIcon ("favorites"_qs), tr ("Bookmarks"));
		tabs->addTab (&Thumbnails_, mgr->GetIcon ("view-preview"_qs), tr ("Thumbnails"));
		tabs->addTab (&Annotations_, mgr->GetIcon ("view-pim-notes"_qs), tr ("Annotations"));
		tabs->addTab (&Search_, mgr->GetIcon ("edit-find"_qs), tr ("Search"));
		tabs->addTab (&OptionalContents_, mgr->GetIcon ("configure"_qs), tr ("Optional contents"));
		setWidget (tabs);

		SetupToc (deps.ViewPosTracker_, deps.LinkContext_);
		SetupThumbnails (deps.ViewPosTracker_, deps.LinkContext_);
		SetupBookmarks ();

		connect (&deps.AnnotationsMgr_,
				&AnnManager::annotationSelected,
				this,
				[this, tabs] { tabs->setCurrentWidget (&Annotations_); });

		auto mw = GetProxyHolder ()->GetRootWindowsManager ()->GetMWProxy (0);
		mw->AddDockWidget (this, { .Area_ = GetLastDockWidgetArea (), .SizeContext_ = "MonocleDockWidget" });
		mw->AssociateDockWidget (this, &deps.TabWidget_);
		mw->ToggleViewActionVisiblity (this, false);
		if (!XmlSettingsManager::Instance ().Property ("DockWidgetVisible", true).toBool ())
			mw->SetDockWidgetVisibility (this, false);
	}

	void Dock::HandleDoc (IDocument& doc)
	{
		if (const auto toc = qobject_cast<IHaveTOC*> (doc.GetQObject ()))
			Toc_.SetTOC (toc->GetTOC ());
		else
			Toc_.SetTOC ({});

		Bookmarks_.HandleDoc (doc);
		Thumbnails_.HandleDoc (doc);
		Search_.Reset ();

		if (const auto ihoc = qobject_cast<IHaveOptionalContent*> (doc.GetQObject ()))
			OptionalContents_.setModel (ihoc->GetOptContentModel ());
		else
			OptionalContents_.setModel (nullptr);
	}

	void Dock::SetupToc (ViewPositionTracker& viewPosTracker, LinkExecutionContext& linkCtx)
	{
		connect (&viewPosTracker,
				&ViewPositionTracker::currentPageChanged,
				&Toc_,
				&TOCWidget::SetCurrentPage);
		connect (&Toc_,
				&TOCWidget::navigationRequested,
				[&linkCtx] (const NavigationAction& act) { linkCtx.Navigate (act); });
	}

	void Dock::SetupThumbnails (ViewPositionTracker& viewPosTracker, LinkExecutionContext& linkCtx)
	{
		connect (&viewPosTracker,
				&ViewPositionTracker::currentPageChanged,
				&Thumbnails_,
				&ThumbsWidget::SetCurrentPage);
		connect (&viewPosTracker,
				&ViewPositionTracker::pagesVisibilityChanged,
				&Thumbnails_,
				&ThumbsWidget::UpdatePagesVisibility);
		connect (&Thumbnails_,
				&ThumbsWidget::pageClicked,
				[&linkCtx] (int page) { linkCtx.Navigate (NavigationAction { page }); });
	}

	void Dock::SetupBookmarks ()
	{
		connect (&Bookmarks_,
				&BookmarksWidget::addBookmarkRequested,
				this,
				&Dock::addBookmarkRequested);
		connect (&Bookmarks_,
				&BookmarksWidget::removeBookmarkRequested,
				this,
				&Dock::removeBookmarkRequested);
		connect (&Bookmarks_,
				&BookmarksWidget::bookmarkActivated,
				this,
				&Dock::bookmarkActivated);
	}
}
