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
#include <QFileDialog>
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
#include <QTreeView>
#include <QUrl>
#include <QFuture>
#include <util/util.h>
#include <util/gui/findnotification.h>
#include <util/gui/menumodeladapter.h>
#include <util/sll/prelude.h>
#include <util/sll/unreachable.h>
#include <interfaces/imwproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include "interfaces/monocle/ihavetoc.h"
#include "interfaces/monocle/isaveabledocument.h"
#include "interfaces/monocle/isearchabledocument.h"
#include "interfaces/monocle/isupportpainting.h"
#include "interfaces/monocle/iknowfileextensions.h"
#include "util/monocle/documentsignals.h"
#include "components/actions/export.h"
#include "components/actions/rotatemenu.h"
#include "components/actions/zoomer.h"
#include "components/gui/dock.h"
#include "components/layout/viewpositiontracker.h"
#include "core.h"
#include "pagegraphicsitem.h"
#include "filewatcher.h"
#include "presenterwidget.h"
#include "recentlyopenedmanager.h"
#include "common.h"
#include "docstatemanager.h"
#include "docinfodialog.h"
#include "xmlsettingsmanager.h"
#include "bookmarkswidget.h"
#include "pageslayoutmanager.h"
#include "textsearchhandler.h"
#include "formmanager.h"
#include "annmanager.h"
#include "linkitem.h"
#include "coreloadproxy.h"
#include "documentbookmarksmanager.h"
#include "pagenumlabel.h"
#include "smoothscroller.h"
#include "xmlsettingsmanager.h"

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

	DocumentTab::TabExecutionContext::TabExecutionContext (DocumentTab& tab)
	: Tab_ { tab }
	{
	}

	void DocumentTab::TabExecutionContext::Navigate (const NavigationAction& act)
	{
		Tab_.Navigate (act);
	}

	void DocumentTab::TabExecutionContext::Navigate (const ExternalNavigationAction& act)
	{
		Tab_.Navigate (act);
	}

	DocumentTab::DocumentTab (const TabClassInfo& tc, QObject *parent)
	: TC_ (tc)
	, ParentPlugin_ (parent)
	, Toolbar_ (new QToolBar ("Monocle"))
	, LayoutManager_ { *new PagesLayoutManager { Ui_.PagesView_, this } }
	, FormManager_ { *new FormManager { Ui_.PagesView_, LinkExecutionContext_ }}
	, AnnManager_ { *new AnnManager { LinkExecutionContext_, this } }
	, DocBMManager_ { *new DocumentBookmarksManager { this, this } }
	, SearchHandler_ { *new TextSearchHandler { this } }
	, ViewPosTracker_ { *new ViewPositionTracker { *Ui_.PagesView_, LayoutManager_, this } }
	, DockWidget_ { std::make_unique<Dock> (Dock::Deps {
			.DocTab_ = *this,
			.AnnotationsMgr_ = AnnManager_,
			.BookmarksMgr_ = DocBMManager_,
			.SearchHandler_ = SearchHandler_,
			.ViewPosTracker_ = ViewPosTracker_,
		}) }
	, NavHistory_ (new NavigationHistory { *this })
	, Zoomer_ { std::make_unique<Zoomer> ([this] { return LayoutManager_.GetCurrentScale (); }) }
	, ScreensaverProhibitor_ (Core::Instance ().GetProxy ()->GetEntityManager ())
	{
		Ui_.PagesView_->setScene (&Scene_);
		Ui_.PagesView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		Scroller_ = new SmoothScroller { *Ui_.PagesView_, this };
		connect (Scroller_,
				&SmoothScroller::isCurrentlyScrollingChanged,
				this,
				[this] (bool isScrolling)
				{
					for (const auto page : Pages_)
						page->SetRenderingEnabled (!isScrolling);
				});

		connect (&*Zoomer_,
				&Zoomer::scaleModeChanged,
				this,
				[this] (ScaleMode mode)
				{
					LayoutManager_.SetScaleMode (mode);
					LayoutManager_.Relayout ();
					scheduleSaveState ();
				});

		connect (&SearchHandler_,
				&TextSearchHandler::navigateRequested,
				this,
				qOverload<const NavigationAction&> (&DocumentTab::Navigate));

		XmlSettingsManager::Instance ().RegisterObject ("InhibitScreensaver", this,
				[this] (const QVariant& val) { ScreensaverProhibitor_.SetProhibitionsEnabled (val.toBool ()); });

		connect (&AnnManager_,
				&AnnManager::navigationRequested,
				Scroller_,
				&SmoothScroller::SmoothCenterOn);

		FindDialog_ = new FindDialog { SearchHandler_, Ui_.PagesView_ };
		FindDialog_->hide ();

		SetupToolbar ();

		new FileWatcher { *this };

		Toolbar_->addSeparator ();
		Toolbar_->addAction (DockWidget_->toggleViewAction ());

		connect (&ViewPosTracker_,
				&ViewPositionTracker::currentPageChanged,
				this,
				[this] (int current)
				{
					PageNumLabel_->SetCurrentPage (current);
					scheduleSaveState ();
				});
		connect (Scroller_,
				&SmoothScroller::isCurrentlyScrollingChanged,
				&ViewPosTracker_,
				[this] (bool scrolling) { ViewPosTracker_.SetUpdatesEnabled (!scrolling); });
	}

	DocumentTab::~DocumentTab () = default;

	ExternalNavigationAction DocumentTab::GetNavigationHistoryEntry () const
	{
		PageRelativePos position;
		auto pageNum = LayoutManager_.GetCurrentPage ();
		if (pageNum >= 0)
			position = Ui_.PagesView_->GetCurrentCenter ().ToPageRelative (*Pages_ [pageNum]);

		return
		{
			CurrentDocPath_,
			{
				pageNum,
				QRectF { position.ToPointF (), QSizeF {} }
			}
		};
	}

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
			return QByteArray ();

		QByteArray result;
		QDataStream out (&result, QIODevice::WriteOnly);
		out << static_cast<quint8> (1)
			<< CurrentDocPath_
			<< LayoutManager_.GetCurrentScale ()
			<< Ui_.PagesView_->GetCurrentCenter ().ToPointF ().toPoint ()
			<< LayoutMode2Name (LayoutManager_.GetLayoutMode ());
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
		if (Core::Instance ().CanLoadDocument (localPath))
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

		SetDoc (url.toLocalFile (), DocumentOpenOptions {});
		event->acceptProposedAction ();
	}

	void DocumentTab::RecoverState (const QByteArray& data)
	{
		QDataStream in (data);
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << "unknown state version" << version;
			return;
		}

		QString path;
		double scale = 0;
		QPoint point;
		QByteArray modeStr;
		in >> path
			>> scale
			>> point
			>> modeStr;

		LayoutManager_.SetLayoutMode (Name2LayoutMode (modeStr));

		SetDoc (path, DocumentOpenOptions {});
		LayoutManager_.SetScaleMode (FixedScale { scale });
		LayoutManager_.Relayout ();

		QTimer::singleShot (0, this, [point, this] { Ui_.PagesView_->centerOn (point); });
	}

	void DocumentTab::ReloadDoc (const QString& doc)
	{
		SetDoc (doc, DocumentOpenOption::IgnoreErrors);
	}

	bool DocumentTab::SetDoc (const QString& path,
			DocumentOpenOptions options,
			const std::optional<NavigationAction>& targetPos)
	{
		if (SaveStateScheduled_)
			saveState ();

		auto document = Core::Instance ().LoadDocument (path);
		if (!document)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to navigate to"
					<< path;
			if (!(options & DocumentOpenOption::IgnoreErrors))
				QMessageBox::critical (this,
						"LeechCraft",
						tr ("Unable to open document %1.")
							.arg ("<em>" + path + "</em>"));
			return false;
		}

		connect (document,
				&CoreLoadProxy::ready,
				this,
				[=, this] (auto ptr, auto path) { HandleLoaderReady (options, ptr, path, targetPos); });

		return true;
	}

	void DocumentTab::SetCurrentPage (int idx)
	{
		Scroller_->SmoothCenterOn (*Pages_ [idx]);
	}

	void DocumentTab::dragEnterEvent (QDragEnterEvent *event)
	{
		HandleDragEnter (event);
	}

	void DocumentTab::dropEvent (QDropEvent *event)
	{
		HandleDrop (event);
	}

	void DocumentTab::SetupToolbarOpen ()
	{
		auto open = new QAction (tr ("Open..."), this);
		open->setProperty ("ActionIcon", "document-open");
		open->setShortcut (QString ("Ctrl+O"));
		connect (open,
				SIGNAL (triggered ()),
				this,
				SLOT (selectFile ()));

		auto roMenu = Core::Instance ().GetROManager ()->CreateOpenMenu (this,
				[this] (const QString& path)
				{
					const QFileInfo fi { path };
					if (!fi.exists ())
						QMessageBox::critical (this,
								"LeechCraft",
								tr ("Seems like file %1 doesn't exist anymore.")
										.arg ("<em>" + fi.fileName () + "</em>"));
					else
						SetDoc (path, DocumentOpenOptions {});
				});

		auto openButton = new QToolButton ();
		openButton->setDefaultAction (open);
		openButton->setMenu (roMenu);
		openButton->setPopupMode (QToolButton::MenuButtonPopup);
		Toolbar_->addWidget (openButton);
	}

	void DocumentTab::SetupToolbarRotate ()
	{
		auto rotateMenu = CreateRotateMenu (AngleNotifier { LayoutManager_, &PagesLayoutManager::rotationUpdated },
				std::bind_front (&PagesLayoutManager::SetRotation, &LayoutManager_));

		auto rotateButton = new QToolButton ();
		rotateButton->setProperty ("ActionIcon", "transform-rotate");
		rotateButton->setMenu (rotateMenu.release ());
		rotateButton->setPopupMode (QToolButton::InstantPopup);

		Toolbar_->addWidget (rotateButton);
	}

	void DocumentTab::SetupToolbarNavigation ()
	{
		{
			auto backButton = new QToolButton;

			const auto backAction = new QAction { tr ("Go back") };
			backAction->setProperty ("ActionIcon", "go-previous");
			backAction->setEnabled (false);
			connect (backAction,
					&QAction::triggered,
					NavHistory_,
					&NavigationHistory::GoBack);
			connect (NavHistory_,
					&NavigationHistory::backwardHistoryAvailabilityChanged,
					backAction,
					&QAction::setEnabled);

			backButton->setDefaultAction (backAction);
			backButton->setMenu (NavHistory_->GetBackwardMenu ());
			backButton->setPopupMode (QToolButton::MenuButtonPopup);
			Toolbar_->addWidget (backButton);
		}

		PageNumLabel_ = new PageNumLabel;
		connect (PageNumLabel_,
				qOverload<int> (&QSpinBox::valueChanged),
				[this] (int value)
				{
					SetCurrentPage (value - 1);
					scheduleSaveState ();
				});
		connect (&LayoutManager_,
				&PagesLayoutManager::layoutModeChanged,
				[this] { PageNumLabel_->setSingleStep (LayoutManager_.GetLayoutModeCount ()); });
		Toolbar_->addWidget (PageNumLabel_);

		{
			auto fwdButton = new QToolButton;

			const auto fwdAction = new QAction { tr ("Go forward") };
			fwdAction->setProperty ("ActionIcon", "go-next");
			fwdAction->setEnabled (false);
			connect (fwdAction,
					&QAction::triggered,
					NavHistory_,
					&NavigationHistory::GoForward);
			connect (NavHistory_,
					&NavigationHistory::forwardHistoryAvailabilityChanged,
					fwdAction,
					&QAction::setEnabled);

			fwdButton->setDefaultAction (fwdAction);
			fwdButton->setMenu (NavHistory_->GetForwardMenu ());
			fwdButton->setPopupMode (QToolButton::MenuButtonPopup);
			Toolbar_->addWidget (fwdButton);
		}
	}

	void DocumentTab::SetupToolbar ()
	{
		SetupToolbarOpen ();

		auto print = new QAction (tr ("Print..."), this);
		print->setProperty ("ActionIcon", "document-print");
		connect (print,
				&QAction::triggered,
				this,
				[this]
				{
					if (CurrentDoc_)
						Print (LayoutManager_.GetCurrentPage (), *CurrentDoc_, *this);
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
				&DocBMManager_,
				&DocumentBookmarksManager::AddBookmark);
		addAction->setProperty ("ActionIcon", "bookmark-new");
		Util::SetMenuModel (*bmMenu, *DocBMManager_.GetModel (),
				[this] (const QModelIndex& idx) { DocBMManager_.Navigate (idx); },
				{
					.AdditionalActions_ = { addAction }
				});
		connect (&DocBMManager_,
				&DocumentBookmarksManager::docAvailable,
				bmMenu,
				&QMenu::setEnabled);
		bmMenu->setEnabled (DocBMManager_.HasDoc ());
		bmButton->setMenu (bmMenu);
		bmButton->setProperty ("ActionIcon", "bookmarks");
		Toolbar_->addWidget (bmButton);

		Toolbar_->addSeparator ();

		AddToolbarEntries (*Toolbar_, Zoomer_->GetToolbarEntries ());

		SetupToolbarRotate ();

		Toolbar_->addSeparator ();

		auto viewGroup = new QActionGroup (this);
		LayOnePage_ = new QAction (tr ("One page"), this);
		LayOnePage_->setProperty ("ActionIcon", "page-simple");
		LayOnePage_->setCheckable (true);
		LayOnePage_->setChecked (true);
		LayOnePage_->setActionGroup (viewGroup);
		connect (LayOnePage_,
				&QAction::triggered,
				[this] { SetLayoutMode (LayoutMode::OnePage); });
		Toolbar_->addAction (LayOnePage_);

		LayTwoPages_ = new QAction (tr ("Two pages"), this);
		LayTwoPages_->setProperty ("ActionIcon", "page-2sides");
		LayTwoPages_->setCheckable (true);
		LayTwoPages_->setActionGroup (viewGroup);
		connect (LayTwoPages_,
				&QAction::triggered,
				[this] { SetLayoutMode (LayoutMode::TwoPages); });
		Toolbar_->addAction (LayTwoPages_);

		LayTwoPagesShifted_ = new QAction (tr ("Two pages (first page separate)"), this);
		LayTwoPagesShifted_->setProperty ("ActionIcon", "page-3sides");
		LayTwoPagesShifted_->setCheckable (true);
		LayTwoPagesShifted_->setActionGroup (viewGroup);
		connect (LayTwoPagesShifted_,
				&QAction::triggered,
				[this] { SetLayoutMode (LayoutMode::TwoPagesShifted); });
		Toolbar_->addAction (LayTwoPagesShifted_);

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

	void DocumentTab::SetLayoutMode (LayoutMode mode)
	{
		LayoutManager_.SetLayoutMode (mode);
		LayoutManager_.Relayout ();

		scheduleSaveState ();
	}

	void DocumentTab::SetPosition (const NavigationAction& nav)
	{
		const auto page = Pages_.value (nav.PageNumber_);
		if (!page)
			return;

		if (const auto& rect = nav.TargetArea_)
		{
			auto center = PageRelativePos { (rect->topLeft () + rect->bottomRight ()) / 2 };
			Scroller_->SmoothCenterOnPoint (center.ToSceneAbsolute (*page));
		}
		else
			SetCurrentPage (nav.PageNumber_);
	}

	void DocumentTab::Navigate (const NavigationAction& nav)
	{
		NavHistory_->SaveCurrentPos ();
		SetPosition (nav);
	}

	void DocumentTab::Navigate (const ExternalNavigationAction& nav)
	{
		NavHistory_->SaveCurrentPos ();
		if (nav.TargetDocument_ == CurrentDocPath_)
		{
			SetPosition (nav.DocumentNavigation_);
			return;
		}

		auto path = nav.TargetDocument_;
		if (QFileInfo { path }.isRelative ())
			path = QFileInfo (CurrentDocPath_).dir ().absoluteFilePath (path);
		SetDoc (path, DocumentOpenOptions {}, nav.DocumentNavigation_);
	}

	void DocumentTab::HandleLoaderReady (DocumentOpenOptions options,
			const IDocument_ptr& document, const QString& path, const std::optional<NavigationAction>& targetPos)
	{
		if (!document || !document->IsValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to navigate to"
					<< path;
			if (!(options & DocumentOpenOption::IgnoreErrors))
				QMessageBox::critical (this,
						"LeechCraft",
						tr ("Unable to open document %1.")
							.arg ("<em>" + path + "</em>"));
			return;
		}

		const auto& state = Core::Instance ().GetDocStateManager ()->GetState (QFileInfo (path).fileName ());

		Core::Instance ().GetROManager ()->RecordOpened (path);

		Scene_.clear ();
		Pages_.clear ();

		CurrentDoc_ = document;
		CurrentDocPath_ = path;
		const auto& title = QFileInfo (path).fileName ();
		emit changeTabName (title);

		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem { *CurrentDoc_, i };
			Scene_.addItem (item);
			Pages_ << item;
		}

		LayoutManager_.HandleDoc (CurrentDoc_.get (), Pages_);
		SearchHandler_.HandleDoc (*CurrentDoc_, Pages_);
		FormManager_.HandleDoc (*CurrentDoc_, Pages_);
		AnnManager_.HandleDoc (*CurrentDoc_, Pages_);
		Ui_.PagesView_->SetDocument (CurrentDoc_.get ());
		PageNumLabel_->SetTotalPageCount (CurrentDoc_->GetNumPages ());

		CreateLinksItems (LinkExecutionContext_, *CurrentDoc_, Pages_);

		emit fileLoaded (path, CurrentDoc_.get (), Pages_);

		recoverDocState (state);

		auto docObj = CurrentDoc_->GetQObject ();

		if (const auto docSignals = CurrentDoc_->GetDocumentSignals ())
		{
			connect (docSignals,
					&DocumentSignals::printRequested,
					this,
					[this] { Print (LayoutManager_.GetCurrentPage (), *CurrentDoc_, *this); });
			connect (docSignals,
					&DocumentSignals::pageContentsChanged,
					this,
					[this] (int idx) { Pages_ [idx]->UpdatePixmap (); });
		}

		emit tabRecoverDataChanged ();

		DocBMManager_.HandleDoc (CurrentDoc_);

		FindAction_->setEnabled (qobject_cast<ISearchableDocument*> (docObj));

		auto saveable = qobject_cast<ISaveableDocument*> (docObj);
		SaveAction_->setEnabled (saveable && saveable->CanSave ().CanSave_);

		ExportPDFAction_->setEnabled (qobject_cast<ISupportPainting*> (docObj));

		if (targetPos)
			Navigate (*targetPos);
	}

	void DocumentTab::saveState ()
	{
		if (!SaveStateScheduled_)
			return;

		SaveStateScheduled_ = false;

		emit tabRecoverDataChanged ();

		if (CurrentDocPath_.isEmpty ())
			return;

		const auto& filename = QFileInfo (CurrentDocPath_).fileName ();
		Core::Instance ().GetDocStateManager ()->SetState (filename,
				{
					LayoutManager_.GetCurrentPage (),
					LayoutManager_.GetLayoutMode (),
					LayoutManager_.GetScaleMode ()
				});
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

	void DocumentTab::selectFile ()
	{
		const auto& extPlugins = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableTo<IKnowFileExtensions*> ();
		QStringList filters;
		QList<QString> allExts;
		for (const auto plugin : extPlugins)
			for (const auto& info : plugin->GetKnownFileExtensions ())
			{
				const auto& mapped = Util::Map (info.Extensions_,
						[] (const QString& str) { return "*." + str; });
				allExts += mapped;
				filters << info.Description_ + " (" + QStringList { mapped }.join (" ") + ")";
			}
		if (!allExts.isEmpty ())
			filters.prepend (tr ("Known files") + " (" + QStringList { allExts }.join (" ") + ")");
		filters << tr ("All files") + " (*.*)";

		const auto& prevPath = XmlSettingsManager::Instance ()
				.Property ("LastOpenFileName", QDir::homePath ()).toString ();
		const auto& path = QFileDialog::getOpenFileName (this,
					tr ("Select file"),
					prevPath,
					filters.join (";;"));
		if (path.isEmpty ())
			return;

		XmlSettingsManager::Instance ()
				.setProperty ("LastOpenFileName", QFileInfo (path).absolutePath ());

		SetDoc (path, DocumentOpenOptions {});
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
		presenter->NavigateTo (LayoutManager_.GetCurrentPage ());
	}

	void DocumentTab::recoverDocState (DocStateManager::State state)
	{
		LayoutManager_.SetLayoutMode (state.Lay_);
		LayoutManager_.SetScaleMode (state.ScaleMode_);
		LayoutManager_.Relayout ();

		Zoomer_->SetScaleMode (state.ScaleMode_);

		Ui_.PagesView_->CenterOn (Ui_.PagesView_->GetViewportTrimmedCenter (*Pages_ [state.CurrentPage_]));

		const auto action = [this]
		{
			switch (LayoutManager_.GetLayoutMode ())
			{
			case LayoutMode::OnePage:
				return LayOnePage_;
			case LayoutMode::TwoPages:
				return LayTwoPages_;
			case LayoutMode::TwoPagesShifted:
				return LayTwoPagesShifted_;
			}

			Util::Unreachable ();
		} ();
		action->setChecked (true);
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

		auto dia = new DocInfoDialog (CurrentDocPath_, CurrentDoc_, this);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}
