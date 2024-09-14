/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "documenttab.h"
#include <functional>
#include <QToolBar>
#include <QMenu>
#include <QToolButton>
#include <QMessageBox>
#include <QtDebug>
#include <QMimeData>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QTimer>
#include <QScrollBar>
#include <QShortcut>
#include <QUrl>
#include <QFuture>
#include <util/util.h>
#include <util/gui/findnotification.h>
#include <util/gui/menumodeladapter.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include "interfaces/monocle/isaveabledocument.h"
#include "interfaces/monocle/isearchabledocument.h"
#include "interfaces/monocle/isupportpainting.h"
#include "interfaces/monocle/iknowfileextensions.h"
#include "util/monocle/documentsignals.h"
#include "components/actions/documentactions.h"
#include "components/actions/export.h"
#include "components/actions/rotatemenu.h"
#include "components/actions/zoomer.h"
#include "components/layout/pageslayoutmanager.h"
#include "components/layout/viewpositiontracker.h"
#include "components/navigation/bookmarksstorage.h"
#include "components/navigation/documentbookmarksmodel.h"
#include "components/navigation/navigator.h"
#include "components/navigation/navigationhistory.h"
#include "components/services/documentloader.h"
#include "components/services/recentlyopenedmanager.h"
#include "components/services/pixmapcachemanager.h"
#include "components/viewitems/linkitem.h"
#include "components/viewitems/pagegraphicsitem.h"
#include "components/viewitems/pagescontextmenuhandler.h"
#include "components/widgets/dock.h"
#include "presenterwidget.h"
#include "common.h"
#include "docinfodialog.h"
#include "xmlsettingsmanager.h"
#include "bookmarkswidget.h"
#include "textsearchhandler.h"
#include "formmanager.h"
#include "annmanager.h"
#include "pagenumlabel.h"
#include "smoothscroller.h"

namespace LC
{
namespace Monocle
{
	class FindDialog final : public Util::FindNotification
	{
		TextSearchHandler& SearchHandler_;
	public:
		FindDialog (TextSearchHandler& searchHandler, QWidget *parent)
		: Util::FindNotification { GetProxyHolder (), parent }
		, SearchHandler_ { searchHandler }
		{
		}
	protected:
		void HandleNext (const QString& text, FindFlags flags) override
		{
			SetSuccessful (SearchHandler_.Search (text, flags));
		}
	};

	struct DocumentTab::Components
	{
		PagesLayoutManager LayoutManager_;
		Navigator Navigator_;

		FormManager FormManager_;
		AnnManager AnnManager_;
		TextSearchHandler SearchHandler_;
		ViewPositionTracker ViewPosTracker_;
		Dock DockWidget_;
		Zoomer Zoomer_;
		SmoothScroller Scroller_;

		DocumentActions Actions_;

		explicit Components (DocumentTab& tab, QToolBar& toolbar, const DocumentTab::Deps& deps)
		: LayoutManager_ { tab.Ui_.PagesView_ }
		, Navigator_ { LayoutManager_, deps.Loader_ }
		, FormManager_ { tab.Ui_.PagesView_, Navigator_.GetNavigationContext () }
		, AnnManager_ { Navigator_.GetNavigationContext () }
		, ViewPosTracker_ { *tab.Ui_.PagesView_, LayoutManager_ }
		, DockWidget_ { {
				.LinkContext_ = Navigator_.GetNavigationContext (),
				.TabWidget_ = tab,
				.AnnotationsMgr_ = AnnManager_,
				.BookmarksStorage_ = deps.BookmarksStorage_,
				.PixmapCacheManager_ = deps.PixmapCacheManager_,
				.SearchHandler_ = SearchHandler_,
				.ViewPosTracker_ = ViewPosTracker_,
		} }
		, Zoomer_ { [this] { return LayoutManager_.GetCurrentScale (); } }
		, Scroller_ { *tab.Ui_.PagesView_ }
		, Actions_ { toolbar, {
				.Navigator_ = Navigator_,
				.RecentlyOpenedManager_ = deps.RecentlyOpenedManager_,
				.DocTabWidget_ = tab,
		} }
		{
		}
	};

	DocumentTab::DocumentTab (const Deps& deps, QObject *parent)
	: TC_ { deps.TC_ }
	, ParentPlugin_ { parent }
	, Toolbar_ { new QToolBar { "Monocle" } }
	, BookmarksStorage_ { deps.BookmarksStorage_ }
	, DocStateManager_ { deps.DocStateManager_ }
	, Loader_ { deps.Loader_ }
	, PixmapCacheManager_ { deps.PixmapCacheManager_ }
	, RecentlyOpenedManager_ { deps.RecentlyOpenedManager_ }
	, C_ { std::make_unique<Components> (*this, *Toolbar_, deps) }
	, ScreensaverProhibitor_ (GetProxyHolder ()->GetEntityManager ())
	{
		Ui_.PagesView_->setScene (&Scene_);
		Ui_.PagesView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		HandlePagesContextMenus (*Ui_.PagesView_, C_->LayoutManager_);

		connect (&C_->Navigator_,
				&Navigator::positionRequested,
				this,
				&DocumentTab::SetPosition);
		connect (&C_->Navigator_,
				&Navigator::loaded,
				this,
				&DocumentTab::HandleDocumentLoaded);
		connect (&C_->Navigator_,
				&Navigator::loadingFailed,
				this,
				[this] (const QString& path)
				{
					QMessageBox::critical (this,
							"Monocle"_qs,
							tr ("Unable to open document %1.").arg ("<em>" + path + "</em>"));
				});

		connect (&C_->Scroller_,
				&SmoothScroller::isCurrentlyScrollingChanged,
				this,
				[this] (bool isScrolling)
				{
					for (const auto page : Pages_)
						page->SetRenderingEnabled (!isScrolling);
				});

		connect (&C_->LayoutManager_,
				&PagesLayoutManager::layoutModeChanged,
				this,
				[this] (LayoutMode mode) { LayoutActions_ [static_cast<int> (mode)]->setChecked (true); });

		connect (&C_->Zoomer_,
				&Zoomer::scaleModeChanged,
				this,
				[this] (ScaleMode mode)
				{
					C_->LayoutManager_.SetScaleMode (mode);
					C_->LayoutManager_.Relayout ();
					scheduleSaveState ();
				});
		connect (&C_->LayoutManager_,
				&PagesLayoutManager::scaleModeChanged,
				&C_->Zoomer_,
				&Zoomer::SetScaleMode);

		connect (&C_->SearchHandler_,
				&TextSearchHandler::navigateRequested,
				&C_->Navigator_,
				&Navigator::Navigate);

		XmlSettingsManager::Instance ().RegisterObject ("InhibitScreensaver", this,
				[this] (const QVariant& val) { ScreensaverProhibitor_.SetProhibitionsEnabled (val.toBool ()); });

		connect (&C_->AnnManager_,
				&AnnManager::navigationRequested,
				&C_->Scroller_,
				&SmoothScroller::SmoothCenterOn);

		FindDialog_ = new FindDialog { C_->SearchHandler_, Ui_.PagesView_ };
		FindDialog_->hide ();

		SetupToolbar ();

		Toolbar_->addSeparator ();
		Toolbar_->addAction (C_->DockWidget_.toggleViewAction ());

		connect (&C_->ViewPosTracker_,
				&ViewPositionTracker::currentPageChanged,
				this,
				[this] (int current)
				{
					PageNumLabel_->SetCurrentPage (current);
					scheduleSaveState ();
				});
		connect (&C_->Scroller_,
				&SmoothScroller::isCurrentlyScrollingChanged,
				&C_->ViewPosTracker_,
				[this] (bool scrolling) { C_->ViewPosTracker_.SetUpdatesEnabled (!scrolling); });

		connect (&C_->DockWidget_,
				&Dock::addBookmarkRequested,
				this,
				&DocumentTab::AddBookmark);
		connect (&C_->DockWidget_,
				&Dock::removeBookmarkRequested,
				this,
				[this] (const Bookmark& bm)
				{
					if (CurrentDoc_)
						BookmarksStorage_.RemoveBookmark (*CurrentDoc_, bm);
				});
		connect (&C_->DockWidget_,
				&Dock::bookmarkActivated,
				this,
				[this] (const Bookmark& bm) { C_->Navigator_.Navigate (bm.ToNavigationAction ()); });
	}

	DocumentTab::~DocumentTab () = default;

	TabClassInfo DocumentTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* DocumentTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	void DocumentTab::Remove ()
	{
		saveState ();
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* DocumentTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void DocumentTab::TabMadeCurrent ()
	{
		ScreensaverProhibitor_.SetProhibited (true);
	}

	void DocumentTab::TabLostCurrent ()
	{
		ScreensaverProhibitor_.SetProhibited (false);
	}

	QString DocumentTab::GetTabRecoverName () const
	{
		return CurrentDocPath_.isEmpty () ?
				QString () :
				"Monocle: " + QFileInfo (CurrentDocPath_).fileName ();
	}

	QIcon DocumentTab::GetTabRecoverIcon () const
	{
		return TC_.Icon_;
	}

	QByteArray DocumentTab::GetTabRecoverData () const
	{
		if (CurrentDocPath_.isEmpty ())
			return {};

		QByteArray result;
		QDataStream out { &result, QIODevice::WriteOnly };
		out << static_cast<quint8> (2)
				<< CurrentDocPath_;
		return result;
	}

	void DocumentTab::FillMimeData (QMimeData *data)
	{
		if (CurrentDocPath_.isEmpty ())
			return;

		data->setUrls ({ QUrl::fromLocalFile (CurrentDocPath_) });
		data->setText (QFileInfo (CurrentDocPath_).fileName ());
	}

	void DocumentTab::HandleDragEnter (QDragMoveEvent *event)
	{
		auto data = event->mimeData ();
		if (!data->hasUrls ())
			return;

		const auto& url = data->urls ().value (0);
		if (!url.isLocalFile () || !QFile::exists (url.toLocalFile ()))
			return;

		const auto& localPath = url.toLocalFile ();
		if (Loader_.CanLoadDocument (localPath))
			event->acceptProposedAction ();
	}

	void DocumentTab::HandleDrop (QDropEvent *event)
	{
		auto data = event->mimeData ();
		if (!data->hasUrls ())
			return;

		const auto& url = data->urls ().value (0);
		if (!url.isLocalFile () || !QFile::exists (url.toLocalFile ()))
			return;

		C_->Navigator_.OpenDocument (url.toLocalFile ());
		event->acceptProposedAction ();
	}

	void DocumentTab::RecoverState (const QByteArray& data)
	{
		QDataStream in { data };
		quint8 version = 0;
		in >> version;
		if (version < 1 || version > 2)
		{
			qWarning () << "unknown state version" << version;
			return;
		}

		QString path;
		in >> path;

		C_->Navigator_.OpenDocument (path);
	}

	void DocumentTab::SetDoc (const QString& path)
	{
		C_->Navigator_.OpenDocument (path);
	}

	void DocumentTab::SetCurrentPage (int idx)
	{
		C_->Scroller_.SmoothCenterOn (*Pages_ [idx]);
	}

	void DocumentTab::dragEnterEvent (QDragEnterEvent *event)
	{
		HandleDragEnter (event);
	}

	void DocumentTab::dropEvent (QDropEvent *event)
	{
		HandleDrop (event);
	}

	void DocumentTab::SetupToolbarRotate ()
	{
		auto rotateMenu = CreateRotateMenu (AngleNotifier { C_->LayoutManager_, &PagesLayoutManager::rotationUpdated },
				[this] (double val, RotationChange ch) { C_->LayoutManager_.SetRotation (val, ch); });

		auto rotateButton = new QToolButton ();
		rotateButton->setProperty ("ActionIcon", "transform-rotate");
		rotateButton->setMenu (rotateMenu.release ());
		rotateButton->setPopupMode (QToolButton::InstantPopup);

		Toolbar_->addWidget (rotateButton);
	}

	void DocumentTab::SetupToolbarNavigation ()
	{
		auto& actions = C_->Navigator_.GetNavigationHistory ().GetActions ();
		{
			auto backButton = new QToolButton;
			backButton->setDefaultAction (&actions.Back_);
			backButton->setMenu (&actions.BackMenu_);
			backButton->setPopupMode (QToolButton::MenuButtonPopup);
			Toolbar_->addWidget (backButton);
		}

		PageNumLabel_ = new PageNumLabel;
		connect (PageNumLabel_,
				qOverload<int> (&QSpinBox::valueChanged),
				[this] (int value) { SetCurrentPage (value - 1); });
		connect (&C_->LayoutManager_,
				&PagesLayoutManager::layoutModeChanged,
				[this] { PageNumLabel_->setSingleStep (C_->LayoutManager_.GetLayoutModeCount ()); });
		Toolbar_->addWidget (PageNumLabel_);

		{
			auto fwdButton = new QToolButton;
			fwdButton->setDefaultAction (&actions.Forward_);
			fwdButton->setMenu (&actions.ForwardMenu_);
			fwdButton->setPopupMode (QToolButton::MenuButtonPopup);
			Toolbar_->addWidget (fwdButton);
		}
	}

	void DocumentTab::SetupToolbar ()
	{
		auto print = new QAction (tr ("Print..."), this);
		print->setProperty ("ActionIcon", "document-print");
		connect (print,
				&QAction::triggered,
				this,
				[this]
				{
					if (CurrentDoc_)
						Print (C_->LayoutManager_.GetCurrentPage (), *CurrentDoc_, *this);
				});
		Toolbar_->addAction (print);

		SaveAction_ = new QAction (tr ("Save"), this);
		SaveAction_->setShortcut (QString ("Ctrl+S"));
		SaveAction_->setProperty ("ActionIcon", "document-save");
		SaveAction_->setEnabled (false);
		connect (SaveAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSave ()));
		Toolbar_->addAction (SaveAction_);

		ExportPDFAction_ = new QAction (tr ("Export as PDF..."), this);
		ExportPDFAction_->setProperty ("ActionIcon", "application-pdf");
		ExportPDFAction_->setEnabled (false);
		connect (ExportPDFAction_,
				&QAction::triggered,
				this,
				[this]
				{
					if (CurrentDoc_)
						ExportToPdf (*CurrentDoc_, *this);
				});
		Toolbar_->addAction (ExportPDFAction_);

		Toolbar_->addSeparator ();

		FindAction_ = new QAction (tr ("Find..."), this);
		FindAction_->setProperty ("ActionIcon", "edit-find");
		FindAction_->setEnabled (false);
		connect (FindAction_,
				SIGNAL (triggered ()),
				FindDialog_,
				SLOT (show ()));
		connect (FindAction_,
				SIGNAL (triggered ()),
				FindDialog_,
				SLOT (setFocus ()));
		Toolbar_->addAction (FindAction_);

		Toolbar_->addSeparator ();

		auto presentation = new QAction (tr ("Presentation..."), this);
		presentation->setProperty ("ActionIcon", "view-presentation");
		connect (presentation,
				SIGNAL (triggered ()),
				this,
				SLOT (handlePresentation ()));
		Toolbar_->addAction (presentation);

		Toolbar_->addSeparator ();

		SetupToolbarNavigation ();

		Toolbar_->addSeparator ();

		auto bmButton = new QToolButton;
		bmButton->setToolTip (tr ("Bookmarks"));
		bmButton->setPopupMode (QToolButton::InstantPopup);
		const auto bmMenu = new QMenu;
		const auto addAction = new QAction { tr ("Add bookmark") };
		connect (addAction,
				&QAction::triggered,
				this,
				&DocumentTab::AddBookmark);
		addAction->setProperty ("ActionIcon", "bookmark-new");
		connect (this,
				&DocumentTab::fileLoaded,
				bmMenu,
				[this, bmMenu, addAction] (const QString&, IDocument *doc)
				{
					BookmarksModel_ = BookmarksStorage_.GetDocumentBookmarksModel (*doc);
					Util::SetMenuModel (*bmMenu, *BookmarksModel_,
							[this] (const QModelIndex& idx)
							{
								C_->Navigator_.Navigate (BookmarksModel_->GetBookmark (idx).ToNavigationAction ());
							},
							{
									.AdditionalActions_ = { addAction }
							});
					bmMenu->setEnabled (true);
				});
		bmMenu->setEnabled (false);
		bmButton->setMenu (bmMenu);
		bmButton->setProperty ("ActionIcon", "bookmarks");
		Toolbar_->addWidget (bmButton);

		Toolbar_->addSeparator ();

		AddToolbarEntries (*Toolbar_, C_->Zoomer_.GetToolbarEntries ());

		SetupToolbarRotate ();

		Toolbar_->addSeparator ();

		auto viewGroup = new QActionGroup (this);
		auto mkAction = [this, viewGroup] (const QString& title, const char *icon, LayoutMode mode)
		{
			auto act = new QAction { title, this };
			act->setProperty ("ActionIcon", icon);
			act->setCheckable (true);
			act->setActionGroup (viewGroup);
			connect (act,
					&QAction::triggered,
					this,
					[this, mode]
					{
						C_->LayoutManager_.SetLayoutMode (mode);
						C_->LayoutManager_.Relayout ();
						scheduleSaveState ();
					});
			Toolbar_->addAction (act);
			return act;
		};
		LayoutActions_ =
		{
			mkAction (tr ("One page"), "page-simple", LayoutMode::OnePage),
			mkAction (tr ("Two pages"), "page-2sides", LayoutMode::TwoPages),
			mkAction (tr ("Two pages (first page separate)"), "page-3sides", LayoutMode::TwoPagesShifted),
		};

		Toolbar_->addSeparator ();

		auto mouseModeGroup = new QActionGroup (this);
		auto moveModeAction = new QAction (tr ("Move mode"), this);
		moveModeAction->setProperty ("ActionIcon", "transform-move");
		moveModeAction->setCheckable (true);
		moveModeAction->setChecked (true);
		moveModeAction->setActionGroup (mouseModeGroup);
		connect (moveModeAction,
				SIGNAL (triggered (bool)),
				this,
				SLOT (setMoveMode (bool)));
		Toolbar_->addAction (moveModeAction);

		auto selectModeAction = new QAction (tr ("Selection mode"), this);
		selectModeAction->setProperty ("ActionIcon", "edit-select");
		selectModeAction->setCheckable (true);
		selectModeAction->setActionGroup (mouseModeGroup);
		connect (selectModeAction,
				SIGNAL (triggered (bool)),
				this,
				SLOT (setSelectionMode (bool)));
		Toolbar_->addAction (selectModeAction);

		Toolbar_->addSeparator ();

		auto infoAction = new QAction (tr ("Document info..."), this);
		infoAction->setProperty ("ActionIcon", "dialog-information");
		connect (infoAction,
				SIGNAL (triggered ()),
				this,
				SLOT (showDocInfo ()));
		Toolbar_->addAction (infoAction);
	}

	void DocumentTab::SetPosition (const NavigationAction& nav)
	{
		const auto page = Pages_.value (nav.PageNumber_);
		if (!page)
			return;

		if (const auto& rect = nav.TargetArea_)
		{
			auto center = (rect->TopLeft<PageRelativePos> () + rect->BottomRight<PageRelativePos> ()) / 2;
			C_->Scroller_.SmoothCenterOnPoint (center.ToSceneAbsolute (*page));
		}
		else
			SetCurrentPage (nav.PageNumber_);
	}

	void DocumentTab::HandleDocumentLoaded (const IDocument_ptr& document, const QString& path)
	{
		saveState ();

		RecentlyOpenedManager_.RecordOpened (path);

		const auto& state = DocStateManager_.GetState (path);

		Scene_.clear ();
		Pages_.clear ();

		CurrentDoc_ = document;
		CurrentDocPath_ = path;
		const auto& title = QFileInfo (path).fileName ();
		emit changeTabName (title);

		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem { *CurrentDoc_, i };
			PixmapCacheManager_.RegisterPage (*item);
			Scene_.addItem (item);
			Pages_ << item;
		}

		C_->LayoutManager_.HandleDoc (CurrentDoc_.get (), Pages_);
		C_->SearchHandler_.HandleDoc (*CurrentDoc_, Pages_);
		C_->FormManager_.HandleDoc (*CurrentDoc_, Pages_);
		C_->AnnManager_.HandleDoc (*CurrentDoc_, Pages_);
		Ui_.PagesView_->SetDocument (CurrentDoc_.get ());
		PageNumLabel_->SetTotalPageCount (CurrentDoc_->GetNumPages ());
		C_->DockWidget_.HandleDoc (*CurrentDoc_);

		CreateLinksItems (C_->Navigator_.GetNavigationContext (), *CurrentDoc_, Pages_);

		emit fileLoaded (path, CurrentDoc_.get (), Pages_);

		recoverDocState (state);

		auto docObj = CurrentDoc_->GetQObject ();

		if (const auto docSignals = CurrentDoc_->GetDocumentSignals ())
		{
			connect (docSignals,
					&DocumentSignals::printRequested,
					this,
					[this] { Print (C_->LayoutManager_.GetCurrentPage (), *CurrentDoc_, *this); });
			connect (docSignals,
					&DocumentSignals::pageContentsChanged,
					this,
					[this] (int idx) { Pages_ [idx]->UpdatePixmap (); });
		}

		emit tabRecoverDataChanged ();

		FindAction_->setEnabled (qobject_cast<ISearchableDocument*> (docObj));

		auto saveable = qobject_cast<ISaveableDocument*> (docObj);
		SaveAction_->setEnabled (saveable && saveable->CanSave ().CanSave_);

		ExportPDFAction_->setEnabled (qobject_cast<ISupportPainting*> (docObj));
	}

	void DocumentTab::saveState ()
	{
		if (!SaveStateScheduled_)
			return;

		SaveStateScheduled_ = false;

		if (CurrentDocPath_.isEmpty ())
			return;

		DocStateManager_.SaveState (CurrentDocPath_,
				{
					C_->LayoutManager_.GetCurrentPagePos (),
					C_->LayoutManager_.GetLayoutMode (),
					C_->LayoutManager_.GetScaleMode (),
				});
	}

	void DocumentTab::AddBookmark ()
	{
		const auto pos = C_->LayoutManager_.GetCurrentPagePos ();
		if (pos && CurrentDoc_)
		{
			const Bookmark bm { tr ("Page %1").arg (pos->Page_ + 1), pos->Page_, pos->Pos_ };
			BookmarksStorage_.AddBookmark (*CurrentDoc_, bm);
		}
		else
			qCritical () << "no current doc or position";
	}

	void DocumentTab::scheduleSaveState ()
	{
		if (SaveStateScheduled_)
			return;

		QTimer::singleShot (5000,
				this,
				SLOT (saveState ()));
		SaveStateScheduled_ = true;
	}

	void DocumentTab::handleSave ()
	{
		if (!CurrentDoc_)
			return;

		auto saveable = qobject_cast<ISaveableDocument*> (CurrentDoc_->GetQObject ());
		if (!saveable)
			return;

		const auto& saveResult = saveable->CanSave ();
		if (!saveResult.CanSave_)
		{
			QMessageBox::critical (this,
					"Monocle",
					tr ("Can't save document: %1.")
						.arg (saveResult.Reason_));
			return;
		}

		saveable->Save (CurrentDocPath_);
	}

	void DocumentTab::handlePresentation ()
	{
		if (!CurrentDoc_)
			return;

		auto presenter = new PresenterWidget (CurrentDoc_);
		presenter->NavigateTo (C_->LayoutManager_.GetCurrentPage ());
	}

	void DocumentTab::recoverDocState (DocStateManager::State state)
	{
		C_->LayoutManager_.SetLayoutMode (state.Lay_);
		C_->LayoutManager_.SetScaleMode (state.ScaleMode_);
		C_->LayoutManager_.Relayout ();

		if (state.CurrentPagePos_)
		{
			const auto& page = *Pages_ [state.CurrentPagePos_->Page_];
			Ui_.PagesView_->CenterOn (state.CurrentPagePos_->Pos_.ToSceneAbsolute (page));
		}
	}

	void DocumentTab::setMoveMode (bool enable)
	{
		if (!enable)
			return;

		Ui_.PagesView_->SetShowReleaseMenu (false);
		Ui_.PagesView_->setDragMode (QGraphicsView::ScrollHandDrag);
	}

	void DocumentTab::setSelectionMode (bool enable)
	{
		if (!enable)
			return;

		Ui_.PagesView_->SetShowReleaseMenu (true);
		Ui_.PagesView_->setDragMode (QGraphicsView::RubberBandDrag);
	}

	void DocumentTab::showDocInfo ()
	{
		if (!CurrentDoc_)
			return;

		auto dia = new DocInfoDialog (CurrentDoc_, this);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}
