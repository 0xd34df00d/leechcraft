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
#include <util/xpc/screensaverprohibitor.h>
#include "interfaces/monocle/idocument.h"
#include "docstatemanager.h"
#include "linkexecutioncontext.h"
#include "common.h"
#include "ui_documenttab.h"

namespace LC
{
namespace Monocle
{
	class DocumentLoader;
	class PageGraphicsItem;
	class FindDialog;
	class PageNumLabel;
	class SmoothScroller;
	class BookmarksStorage;
	class DocumentBookmarksModel;

	class DocumentTab : public QWidget
					  , public ITabWidget
					  , public IRecoverableTab
					  , public IDNDTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab IDNDTab)

		Ui::DocumentTab Ui_;
		Util::UiInit UiInit_ { Ui_, *this };

		QGraphicsScene Scene_;

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

		BookmarksStorage& BookmarksStorage_;
		DocumentLoader& Loader_;

		std::shared_ptr<DocumentBookmarksModel> BookmarksModel_;

		struct Components;
		std::unique_ptr<Components> C_;

		IDocument_ptr CurrentDoc_;
		QString CurrentDocPath_;
		QVector<PageGraphicsItem*> Pages_;

		bool SaveStateScheduled_ = false;

		Util::ScreensaverProhibitor ScreensaverProhibitor_;
	public:
		DocumentTab (BookmarksStorage&, DocumentLoader&, const TabClassInfo&, QObject*);
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

		void SetDoc (const QString&);

		void SetCurrentPage (int);
	protected:
		void dragEnterEvent (QDragEnterEvent*) override;
		void dropEvent (QDropEvent*) override;
	private:
		void SetPosition (const NavigationAction&);

		void SetupToolbarOpen ();
		void SetupToolbarRotate ();
		void SetupToolbarNavigation ();
		void SetupToolbar ();

		void SetLayoutMode (LayoutMode);

		void HandleDocumentLoaded (const IDocument_ptr&, const QString&);

		void AddBookmark ();
	private slots:
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
	};
}
}
