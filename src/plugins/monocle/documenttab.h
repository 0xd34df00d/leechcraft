/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/idndtab.h>
#include <util/gui/uiinit.h>
#include <util/sll/bitflags.h>
#include <util/xpc/screensaverprohibitor.h>
#include "interfaces/monocle/idocument.h"
#include "docstatemanager.h"
#include "navigationhistory.h"
#include "common.h"
#include "ui_documenttab.h"

class QTreeView;
class QMenu;

namespace LC
{
namespace Monocle
{
	class PagesLayoutManager;
	class PageGraphicsItem;
	class TextSearchHandler;
	class FindDialog;
	class FormManager;
	class LinksManager;
	class AnnManager;
	class DocumentBookmarksManager;
	class PageNumLabel;
	class SmoothScroller;
	class Zoomer;
	class Dock;

	class DocumentTab : public QWidget
					  , public ITabWidget
					  , public IRecoverableTab
					  , public IDNDTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab IDNDTab)

		Ui::DocumentTab Ui_;
		Util::UiInit UiInit_ { Ui_, *this };

		TabClassInfo TC_;
		QObject *ParentPlugin_;

		QToolBar *Toolbar_;
		PageNumLabel *PageNumLabel_ = nullptr;

		QAction *LayOnePage_ = nullptr;
		QAction *LayTwoPages_ = nullptr;
		QAction *LayTwoPagesShifted_ = nullptr;

		QAction *SaveAction_ = nullptr;
		QAction *ExportPDFAction_ = nullptr;
		QAction *FindAction_ = nullptr;
		FindDialog *FindDialog_ = nullptr;

		SmoothScroller *Scroller_ = nullptr;

		PagesLayoutManager *LayoutManager_ = nullptr;
		FormManager *FormManager_ = nullptr;
		LinksManager *LinksManager_ = nullptr;
		AnnManager& AnnManager_;
		DocumentBookmarksManager& DocBMManager_;
		TextSearchHandler& SearchHandler_;

		Dock *DockWidget_ = nullptr;

		NavigationHistory * const NavHistory_;

		std::unique_ptr<Zoomer> Zoomer_;

		IDocument_ptr CurrentDoc_;
		QString CurrentDocPath_;
		QVector<PageGraphicsItem*> Pages_;
		QGraphicsScene Scene_;

		bool SaveStateScheduled_ = false;

		int PrevCurrentPage_;

		NavigationAction Onload_ { -1, {} };

		Util::ScreensaverProhibitor ScreensaverProhibitor_;
	public:
		enum class DocumentOpenOption : std::uint8_t
		{
			None = 0x0,
			IgnoreErrors = 0x1
		};
		using DocumentOpenOptions = Util::BitFlags<DocumentOpenOption>;

		DocumentTab (const TabClassInfo&, QObject*);
		~DocumentTab () override;

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;
		QToolBar* GetToolBar () const override;
		void TabMadeCurrent () override;
		void TabLostCurrent () override;

		QString GetTabRecoverName () const override;
		QIcon GetTabRecoverIcon () const override;
		QByteArray GetTabRecoverData () const override;

		void FillMimeData (QMimeData*) override;
		void HandleDragEnter (QDragMoveEvent*) override;
		void HandleDrop (QDropEvent*) override;

		void RecoverState (const QByteArray&);

		void ReloadDoc (const QString&);
		bool SetDoc (const QString&, DocumentOpenOptions);

		int GetCurrentPage () const;
		void SetCurrentPage (int, bool immediate = false);

		QPoint GetCurrentCenter () const;
		void CenterOn (const QPoint&);

		void Navigate (const NavigationAction&);
		void Navigate (const ExternalNavigationAction&);

		ExternalNavigationAction GetNavigationHistoryEntry () const;
	protected:
		void dragEnterEvent (QDragEnterEvent*) override;
		void dropEvent (QDropEvent*) override;
	private:
		void SetPosition (const NavigationAction&);

		void SetupToolbarOpen ();
		void SetupToolbarRotate ();
		void SetupToolbarNavigation ();
		void SetupToolbar ();

		QPoint GetViewportCenter () const;
		void Relayout ();
		void SetLayoutMode (LayoutMode);

		void RegenPageVisibility ();

		void CheckCurrentPageChange ();
	private slots:
		void handleLoaderReady (DocumentOpenOptions, const IDocument_ptr&, const QString&);

		void scheduleSaveState ();
		void saveState ();

		void selectFile ();
		void handleSave ();

		void handlePresentation ();

		void recoverDocState (DocStateManager::State);

		void setMoveMode (bool);
		void setSelectionMode (bool);

		void showDocInfo ();
	signals:
		void changeTabName (const QString&) override;
		void removeTab () override;

		void tabRecoverDataChanged () override;

		void fileLoaded (const QString& path, IDocument *doc, const QVector<PageGraphicsItem*>& pages);

		void currentPageChanged (int);
		void pagesVisibilityChanged (const QMap<int, QRect>&);
	};
}
}
