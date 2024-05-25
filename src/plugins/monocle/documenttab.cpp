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
#include <QDockWidget>
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
#include "interfaces/monocle/ihaveoptionalcontent.h"
#include "util/monocle/documentsignals.h"
#include "components/actions/export.h"
#include "components/actions/rotatemenu.h"
#include "components/actions/zoomer.h"
#include "core.h"
#include "pagegraphicsitem.h"
#include "filewatcher.h"
#include "tocwidget.h"
#include "presenterwidget.h"
#include "recentlyopenedmanager.h"
#include "common.h"
#include "docstatemanager.h"
#include "docinfodialog.h"
#include "xmlsettingsmanager.h"
#include "bookmarkswidget.h"
#include "pageslayoutmanager.h"
#include "thumbswidget.h"
#include "textsearchhandler.h"
#include "formmanager.h"
#include "annmanager.h"
#include "linksmanager.h"
#include "annwidget.h"
#include "coreloadproxy.h"
#include "core.h"
#include "searchtabwidget.h"
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
		TextSearchHandler * const SearchHandler_;
	public:
		FindDialog (TextSearchHandler *searchHandler, QWidget *parent)
		: Util::FindNotification (Core::Instance ().GetProxy (), parent)
		, SearchHandler_ (searchHandler)
		{
		}
	protected:
		void HandleNext (const QString& text, FindFlags flags) override
		{
			SetSuccessful (SearchHandler_->Search (text, flags));
		}
	};

	DocumentTab::DocumentTab (const TabClassInfo& tc, QObject *parent)
	: TC_ (tc)
	, ParentPlugin_ (parent)
	, Toolbar_ (new QToolBar ("Monocle"))
	, DocBMManager_ { *new DocumentBookmarksManager { this, this } }
	, DockWidget_ (new QDockWidget (tr ("Monocle dock")))
	, TOCWidget_ (new TOCWidget ())
	, BMWidget_ (new BookmarksWidget (DocBMManager_))
	, ThumbsWidget_ (new ThumbsWidget ())
	, OptContentsWidget_ (new QTreeView)
	, NavHistory_ (new NavigationHistory { *this })
	, Zoomer_ { std::make_unique<Zoomer> ([this] { return LayoutManager_->GetCurrentScale (); }) }
	, ScreensaverProhibitor_ (Core::Instance ().GetProxy ()->GetEntityManager ())
	{
		Ui_.PagesView_->setScene (&Scene_);
		Ui_.PagesView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		Scroller_ = new SmoothScroller { Ui_.PagesView_, this };
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
					LayoutManager_->SetScaleMode (mode);
					Relayout ();
					scheduleSaveState ();
				});

		LayoutManager_ = new PagesLayoutManager (Ui_.PagesView_, Scroller_, this);
		SearchHandler_ = new TextSearchHandler (this);
		connect (SearchHandler_,
				&TextSearchHandler::navigateRequested,
				this,
				qOverload<const NavigationAction&> (&DocumentTab::Navigate));

		XmlSettingsManager::Instance ().RegisterObject ("InhibitScreensaver", this,
				[this] (const QVariant& val) { ScreensaverProhibitor_.SetProhibitionsEnabled (val.toBool ()); });

		FormManager_ = new FormManager (Ui_.PagesView_, *this);
		AnnManager_ = new AnnManager (Scroller_, *this);
		LinksManager_ = new LinksManager (*this);

		AnnWidget_ = new AnnWidget (AnnManager_);

		SearchTabWidget_ = new SearchTabWidget (SearchHandler_);

		FindDialog_ = new FindDialog (SearchHandler_, Ui_.PagesView_);
		FindDialog_->hide ();

		SetupToolbar ();

		new FileWatcher (this);

		const auto mgr = Core::Instance ().GetProxy ()->GetIconThemeManager ();
		const auto& tocIcon = mgr->GetIcon ("view-table-of-contents-ltr");

		auto dockTabWidget = new QTabWidget;
		dockTabWidget->setTabPosition (QTabWidget::West);
		dockTabWidget->addTab (TOCWidget_, tocIcon, tr ("Table of contents"));
		dockTabWidget->addTab (BMWidget_, mgr->GetIcon ("favorites"), tr ("Bookmarks"));
		dockTabWidget->addTab (ThumbsWidget_, mgr->GetIcon ("view-preview"), tr ("Thumbnails"));
		dockTabWidget->addTab (AnnWidget_, mgr->GetIcon ("view-pim-notes"), tr ("Annotations"));
		dockTabWidget->addTab (SearchTabWidget_, mgr->GetIcon ("edit-find"), tr ("Search"));
		dockTabWidget->addTab (OptContentsWidget_, mgr->GetIcon ("configure"), tr ("Optional contents"));

		connect (AnnManager_,
				&AnnManager::annotationSelected,
				[this, dockTabWidget] { dockTabWidget->setCurrentWidget (AnnWidget_); });

		connect (ThumbsWidget_,
				&ThumbsWidget::pageClicked,
				[this] (int num) { SetCurrentPage (num); });

		DockWidget_->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
		DockWidget_->setAllowedAreas (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		DockWidget_->setWidget (dockTabWidget);

		DockWidget_->setWindowIcon (tocIcon);
		DockWidget_->toggleViewAction ()->setIcon (tocIcon);

		Toolbar_->addSeparator ();
		Toolbar_->addAction (DockWidget_->toggleViewAction ());

		auto dwa = static_cast<Qt::DockWidgetArea> (XmlSettingsManager::Instance ()
				.Property ("DockWidgetArea", Qt::RightDockWidgetArea).toInt ());
		if (dwa == Qt::NoDockWidgetArea)
			dwa = Qt::RightDockWidgetArea;

		auto mw = Core::Instance ().GetProxy ()->GetRootWindowsManager ()->GetMWProxy (0);
		mw->AddDockWidget (DockWidget_, { .Area_ = dwa, .SizeContext_ = "MonocleDockWidget" });
		mw->AssociateDockWidget (DockWidget_, this);
		mw->ToggleViewActionVisiblity (DockWidget_, false);
		if (!XmlSettingsManager::Instance ().Property ("DockWidgetVisible", true).toBool ())
			mw->SetDockWidgetVisibility (DockWidget_, false);
		connect (DockWidget_,
				&QDockWidget::dockLocationChanged,
				[] (Qt::DockWidgetArea area)
				{
					if (area != Qt::AllDockWidgetAreas && area != Qt::NoDockWidgetArea)
						XmlSettingsManager::Instance ().setProperty ("DockWidgetArea", area);
				});
		connect (DockWidget_,
				&QDockWidget::visibilityChanged,
				[] (bool visible) { XmlSettingsManager::Instance ().setProperty ("DockWidgetVisible", visible); });

		connect (this,
				&DocumentTab::currentPageChanged,
				PageNumLabel_,
				&PageNumLabel::SetCurrentPage);
		connect (Ui_.PagesView_->verticalScrollBar (),
				&QScrollBar::valueChanged,
				this,
				&DocumentTab::CheckCurrentPageChange);
		connect (Scroller_,
				&SmoothScroller::isCurrentlyScrollingChanged,
				[this] (bool isScrolling)
				{
					if (!isScrolling)
						CheckCurrentPageChange ();
				});
		connect (this,
				&DocumentTab::currentPageChanged,
				this,
				&DocumentTab::scheduleSaveState);

		connect (this,
				SIGNAL (currentPageChanged (int)),
				ThumbsWidget_,
				SLOT (handleCurrentPage (int)));
		connect (this,
				&DocumentTab::currentPageChanged,
				TOCWidget_,
				&TOCWidget::SetCurrentPage);
		connect (TOCWidget_,
				&TOCWidget::navigationRequested,
				this,
				qOverload<const NavigationAction&> (&DocumentTab::Navigate));
		connect (this,
				SIGNAL (pagesVisibilityChanged (QMap<int, QRect>)),
				ThumbsWidget_,
				SLOT (updatePagesVisibility (QMap<int, QRect>)));
	}

	DocumentTab::~DocumentTab () = default;

	ExternalNavigationAction DocumentTab::GetNavigationHistoryEntry () const
	{
		QPointF position;
		auto pageNum = LayoutManager_->GetCurrentPage ();
		if (pageNum >= 0)
		{
			const auto page = Pages_.value (pageNum);
			const auto& size = page->boundingRect ().size ();
			position = page->mapFromScene (Ui_.PagesView_->GetCurrentCenter ());
			position.rx () /= size.width ();
			position.ry () /= size.height ();

			if (position.rx () > 1 && LayoutManager_->GetLayoutModeCount () == 2)
			{
				--position.rx ();
				++pageNum;
			}
		}

		return
		{
			CurrentDocPath_,
			{
				pageNum,
				QRectF { position, QSizeF {} }
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
		emit removeTab ();

		Scene_.clear ();

		delete TOCWidget_;
		delete BMWidget_;
		delete ThumbsWidget_;
		delete DockWidget_->widget ();
		delete DockWidget_;

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
			<< LayoutManager_->GetCurrentScale ()
			<< Ui_.PagesView_->mapToScene (GetViewportCenter ()).toPoint ()
			<< LayoutMode2Name (LayoutManager_->GetLayoutMode ());
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

		LayoutManager_->SetLayoutMode (Name2LayoutMode (modeStr));

		SetDoc (path, DocumentOpenOptions {});
		LayoutManager_->SetScaleMode (FixedScale { scale });
		Relayout ();

		QTimer::singleShot (0, this, [point, this] { CenterOn (point); });
	}

	void DocumentTab::ReloadDoc (const QString& doc)
	{
		SetDoc (doc, DocumentOpenOption::IgnoreErrors);
	}

	bool DocumentTab::SetDoc (const QString& path, DocumentOpenOptions options)
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
				[options, this] (auto ptr, auto path) { handleLoaderReady (options, ptr, path); });

		return true;
	}

	int DocumentTab::GetCurrentPage () const
	{
		return LayoutManager_->GetCurrentPage ();
	}

	void DocumentTab::SetCurrentPage (int idx, bool immediate)
	{
		LayoutManager_->SetCurrentPage (idx, immediate);
	}

	QPoint DocumentTab::GetCurrentCenter () const
	{
		return Ui_.PagesView_->GetCurrentCenter ().toPoint ();
	}

	void DocumentTab::CenterOn (const QPoint& point)
	{
		Scroller_->SmoothCenterOn (point.x (), point.y ());
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
		auto rotateMenu = CreateRotateMenu (AngleNotifier { *LayoutManager_, &PagesLayoutManager::rotationUpdated },
				std::bind_front (&PagesLayoutManager::SetRotation, LayoutManager_));

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
		connect (LayoutManager_,
				&PagesLayoutManager::layoutModeChanged,
				[this] { PageNumLabel_->setSingleStep (LayoutManager_->GetLayoutModeCount ()); });
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
						Print (LayoutManager_->GetCurrentPage (), *CurrentDoc_, *this);
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
		bmButton->setMenu (DocBMManager_.GetMenu ());
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

	QPoint DocumentTab::GetViewportCenter () const
	{
		return LayoutManager_->GetViewportCenter ();
	}

	void DocumentTab::Relayout ()
	{
		if (!CurrentDoc_)
			return;

		LayoutManager_->Relayout ();

		if (Onload_.PageNumber_ >= 0)
		{
			SetPosition (Onload_);
			Onload_.PageNumber_ = -1;
		}

		CheckCurrentPageChange ();
	}

	void DocumentTab::SetLayoutMode (LayoutMode mode)
	{
		LayoutManager_->SetLayoutMode (mode);
		Relayout ();

		scheduleSaveState ();
	}

	void DocumentTab::RegenPageVisibility ()
	{
		if (receivers (SIGNAL (pagesVisibilityChanged (QMap<int, QRect>))) <= 0)
			return;

		const auto& viewRect = Ui_.PagesView_->viewport ()->rect ();
		const auto& visibleRect = Ui_.PagesView_->mapToScene (viewRect);

		QMap<int, QRect> rects;
		for (auto item : Ui_.PagesView_->items (viewRect))
		{
			auto page = dynamic_cast<PageGraphicsItem*> (item);
			if (!page)
				continue;

			const auto& pageRect = page->mapToScene (page->boundingRect ());
			const auto& xsect = visibleRect.intersected (pageRect);
			const auto& pageXsect = page->MapToDoc (page->mapFromScene (xsect).boundingRect ());
			rects [page->GetPageNum ()] = pageXsect.toAlignedRect ();
		}

		emit pagesVisibilityChanged (rects);
	}

	void DocumentTab::SetPosition (const NavigationAction& nav)
	{
		const auto page = Pages_.value (nav.PageNumber_);
		if (!page)
			return;

		if (const auto& rect = nav.TargetArea_)
		{
			const auto& renderedSize = page->boundingRect ().size ();

			auto center = (rect->topLeft () + rect->bottomRight ()) / 2;
			center.rx () *= renderedSize.width ();
			center.ry () *= renderedSize.height ();
			const auto& mapped = page->mapToScene (center);

			Scroller_->SmoothCenterOn (mapped.x (), mapped.y ());
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

		Onload_ = nav.DocumentNavigation_;
		if (!SetDoc (path, DocumentOpenOptions {}))
			Onload_.PageNumber_ = -1;
	}

	void DocumentTab::handleLoaderReady (DocumentOpenOptions options,
			const IDocument_ptr& document, const QString& path)
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
			auto item = new PageGraphicsItem (CurrentDoc_, i);
			Scene_.addItem (item);
			Pages_ << item;
		}

		LayoutManager_->HandleDoc (CurrentDoc_, Pages_);
		SearchHandler_->HandleDoc (CurrentDoc_, Pages_);
		FormManager_->HandleDoc (CurrentDoc_, Pages_);
		AnnManager_->HandleDoc (CurrentDoc_, Pages_);
		LinksManager_->HandleDoc (CurrentDoc_, Pages_);
		Ui_.PagesView_->SetDocument (CurrentDoc_.get ());
		PageNumLabel_->SetTotalPageCount (CurrentDoc_->GetNumPages ());

		recoverDocState (state);
		Relayout ();
		SetCurrentPage (state.CurrentPage_, true);

		CheckCurrentPageChange ();

		auto docObj = CurrentDoc_->GetQObject ();

		auto toc = qobject_cast<IHaveTOC*> (docObj);
		TOCWidget_->SetTOC (toc ? toc->GetTOC () : TOCEntryLevel_t ());

		if (const auto docSignals = CurrentDoc_->GetDocumentSignals ())
		{
			connect (docSignals,
					&DocumentSignals::printRequested,
					this,
					[this] { Print (LayoutManager_->GetCurrentPage (), *CurrentDoc_, *this); });
			connect (docSignals,
					&DocumentSignals::pageContentsChanged,
					this,
					[this] (int idx) { Pages_ [idx]->UpdatePixmap (); });
		}

		emit fileLoaded (path, CurrentDoc_.get (), Pages_);

		emit tabRecoverDataChanged ();

		DocBMManager_.HandleDoc (CurrentDoc_);
		ThumbsWidget_->HandleDoc (CurrentDoc_);
		SearchTabWidget_->HandleDoc (CurrentDoc_);

		if (const auto ihoc = qobject_cast<IHaveOptionalContent*> (docObj))
			OptContentsWidget_->setModel (ihoc->GetOptContentModel ());
		else
			OptContentsWidget_->setModel (nullptr);

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

		emit tabRecoverDataChanged ();

		if (CurrentDocPath_.isEmpty ())
			return;

		const auto& filename = QFileInfo (CurrentDocPath_).fileName ();
		Core::Instance ().GetDocStateManager ()->SetState (filename,
				{
					LayoutManager_->GetCurrentPage (),
					LayoutManager_->GetLayoutMode (),
					LayoutManager_->GetScaleMode ()
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
		presenter->NavigateTo (LayoutManager_->GetCurrentPage ());
	}

	void DocumentTab::CheckCurrentPageChange ()
	{
		if (Scroller_->IsCurrentlyScrolling ())
			return;

		RegenPageVisibility ();

		auto current = LayoutManager_->GetCurrentPage ();
		if (PrevCurrentPage_ == current)
			return;

		PrevCurrentPage_ = current;
		emit currentPageChanged (current);
	}

	void DocumentTab::recoverDocState (DocStateManager::State state)
	{
		LayoutManager_->SetLayoutMode (state.Lay_);
		LayoutManager_->SetScaleMode (state.ScaleMode_);
		Zoomer_->SetScaleMode (state.ScaleMode_);

		const auto action = [this]
		{
			switch (LayoutManager_->GetLayoutMode ())
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
