/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "documenttab.h"
#include <functional>
#include <QToolBar>
#include <QComboBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QLineEdit>
#include <QMenu>
#include <QToolButton>
#include <QPrinter>
#include <QPrintDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QClipboard>
#include <QtDebug>
#include <QTimer>
#include <QMimeData>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QImageWriter>
#include <QUrl>
#include <util/util.h>
#include <interfaces/imwproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "interfaces/monocle/ihavetoc.h"
#include "interfaces/monocle/ihavetextcontent.h"
#include "interfaces/monocle/isupportannotations.h"
#include "interfaces/monocle/idynamicdocument.h"
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

namespace LeechCraft
{
namespace Monocle
{
	const int Margin = 10;

	DocumentTab::DocumentTab (const TabClassInfo& tc, QObject *parent)
	: TC_ (tc)
	, ParentPlugin_ (parent)
	, Toolbar_ (new QToolBar ("Monocle"))
	, ScalesBox_ (0)
	, PageNumLabel_ (0)
	, DockWidget_ (0)
	, TOCWidget_ (new TOCWidget ())
	, BMWidget_ (new BookmarksWidget ())
	, LayMode_ (LayoutMode::OnePage)
	, MouseMode_ (MouseMode::Move)
	, RelayoutScheduled_ (true)
	, SaveStateScheduled_ (false)
	, Onload_ ({ -1, 0, 0 })
	{
		Ui_.setupUi (this);
		Ui_.PagesView_->setScene (&Scene_);
		Ui_.PagesView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		SetupToolbar ();

		new FileWatcher (this);

		auto proxy = Core::Instance ().GetProxy ();
		const auto& tocIcon = proxy->GetIcon ("view-table-of-contents-ltr");

		auto dockTabWidget = new QTabWidget;
		dockTabWidget->setTabPosition (QTabWidget::West);
		dockTabWidget->addTab (TOCWidget_, tocIcon, tr ("Table of contents"));
		dockTabWidget->addTab (BMWidget_,
				proxy->GetIcon ("favorites"),
				tr ("Bookmarks"));

		DockWidget_ = new QDockWidget (tr ("Monocle dock"));
		DockWidget_->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
		DockWidget_->setWidget (dockTabWidget);

		DockWidget_->setWindowIcon (tocIcon);
		DockWidget_->toggleViewAction ()->setIcon (tocIcon);

		Toolbar_->addSeparator ();
		Toolbar_->addAction (DockWidget_->toggleViewAction ());

		auto mw = Core::Instance ().GetProxy ()->GetRootWindowsManager ()->GetMWProxy (0);
		mw->AddDockWidget (Qt::RightDockWidgetArea, DockWidget_);
		mw->AssociateDockWidget (DockWidget_, this);
		mw->ToggleViewActionVisiblity (DockWidget_, false);

		connect (Ui_.PagesView_,
				SIGNAL (sizeChanged ()),
				this,
				SLOT (scheduleRelayout ()),
				Qt::QueuedConnection);
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
		delete DockWidget_;
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* DocumentTab::GetToolBar () const
	{
		return Toolbar_;
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
			<< GetCurrentScale ()
			<< Ui_.PagesView_->mapToScene (GetViewportCenter ()).toPoint ();

		switch (LayMode_)
		{
		case LayoutMode::OnePage:
			out << QByteArray ("one");
			break;
		case LayoutMode::TwoPages:
			out << QByteArray ("two");
			break;
		}

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

		const auto& url = data->urls ().first ();
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

		const auto& url = data->urls ().first ();
		if (!url.isLocalFile () || !QFile::exists (url.toLocalFile ()))
			return;

		SetDoc (url.toLocalFile ());
		event->acceptProposedAction ();
	}

	void DocumentTab::RecoverState (const QByteArray& data)
	{
		QDataStream in (data);
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown state version"
					<< version;
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

		if (modeStr == "one")
			LayMode_ = LayoutMode::OnePage;
		else if (modeStr == "two")
			LayMode_ = LayoutMode::TwoPages;

		SetDoc (path);
		Relayout (scale);
		QMetaObject::invokeMethod (this,
				"delayedCenterOn",
				Qt::QueuedConnection,
				Q_ARG (QPoint, point));
	}

	void DocumentTab::ReloadDoc (const QString& doc)
	{
		Scene_.clear ();
		Pages_.clear ();
		CurrentDoc_ = IDocument_ptr ();
		CurrentDocPath_.clear ();

		const auto& pos = Ui_.PagesView_->GetCurrentCenter ();

		SetDoc (doc);

		if (Scene_.itemsBoundingRect ().contains (pos))
			Ui_.PagesView_->centerOn (pos);
	}

	bool DocumentTab::SetDoc (const QString& path)
	{
		if (SaveStateScheduled_)
			saveState ();

		auto document = Core::Instance ().LoadDocument (path);
		if (!document || !document->IsValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to navigate to"
					<< path;
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("Unable to open document %1.")
						.arg ("<em>" + path + "</em>"));
			return false;
		}

		const auto& state = Core::Instance ()
				.GetDocStateManager ()->GetState (QFileInfo (path).fileName ());

		Core::Instance ().GetROManager ()->RecordOpened (path);

		Scene_.clear ();
		Pages_.clear ();

		CurrentDoc_ = document;
		CurrentDocPath_ = path;
		const auto& title = QFileInfo (path).fileName ();
		emit changeTabName (this, title);

		auto isa = qobject_cast<ISupportAnnotations*> (CurrentDoc_->GetObject ());

		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem (CurrentDoc_, i);
			Scene_.addItem (item);
			Pages_ << item;

			if (isa)
				isa->GetAnnotations (i);
		}

		LayMode_ = state.Lay_;
		Relayout (state.CurrentScale_ > 0 ? state.CurrentScale_ : GetCurrentScale ());
		SetCurrentPage (state.CurrentPage_, 0);

		updateNumLabel ();

		TOCEntryLevel_t topLevel;
		if (auto toc = qobject_cast<IHaveTOC*> (CurrentDoc_->GetObject ()))
			topLevel = toc->GetTOC ();
		TOCWidget_->SetTOC (topLevel);
		TOCWidget_->setEnabled (!topLevel.isEmpty ());

		connect (CurrentDoc_->GetObject (),
				SIGNAL (navigateRequested (QString, int, double, double)),
				this,
				SLOT (handleNavigateRequested (QString, int, double, double)),
				Qt::QueuedConnection);

		emit fileLoaded (path);

		emit tabRecoverDataChanged ();

		if (qobject_cast<IDynamicDocument*> (CurrentDoc_->GetObject ()))
		{
			connect (CurrentDoc_->GetObject (),
					SIGNAL (pageSizeChanged (int)),
					this,
					SLOT (handlePageSizeChanged (int)));
			connect (CurrentDoc_->GetObject (),
					SIGNAL (pageContentsChanged (int)),
					this,
					SLOT (handlePageContentsChanged (int)));
		}

		return true;
	}

	int DocumentTab::GetCurrentPage () const
	{
		const auto& center = GetViewportCenter ();
		auto item = Ui_.PagesView_->itemAt (center - QPoint (1, 1));
		if (!item)
			item = Ui_.PagesView_->itemAt (center - QPoint (Margin, Margin));
		auto pos = std::find_if (Pages_.begin (), Pages_.end (),
				[item] (decltype (Pages_.front ()) e) { return e == item; });
		return pos == Pages_.end () ? -1 : std::distance (Pages_.begin (), pos);
	}

	void DocumentTab::SetCurrentPage (int idx, bool immediate)
	{
		if (idx < 0 || idx >= Pages_.size ())
			return;

		auto page = Pages_.at (idx);
		const auto& rect = page->boundingRect ();
		const auto& pos = page->scenePos ();
		int xCenter = pos.x () + rect.width () / 2;
		const auto visibleHeight = std::min (static_cast<int> (rect.height ()),
				Ui_.PagesView_->viewport ()->contentsRect ().height ());
		int yCenter = pos.y () + visibleHeight / 2;

		if (immediate)
			Ui_.PagesView_->centerOn (xCenter, yCenter);
		else
			Ui_.PagesView_->SmoothCenterOn (xCenter, yCenter);
	}

	QPoint DocumentTab::GetCurrentCenter () const
	{
		return Ui_.PagesView_->GetCurrentCenter ();
	}

	void DocumentTab::CenterOn (const QPoint& point)
	{
		Ui_.PagesView_->SmoothCenterOn (point.x (), point.y ());
	}

	void DocumentTab::SetupToolbar ()
	{
		auto open = new QAction (tr ("Open..."), this);
		open->setProperty ("ActionIcon", "document-open");
		open->setShortcut (QString ("Ctrl+O"));
		connect (open,
				SIGNAL (triggered ()),
				this,
				SLOT (selectFile ()));

		auto roMenu = Core::Instance ().GetROManager ()->CreateOpenMenu (this);
		connect (roMenu,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleRecentOpenAction (QAction*)));

		auto openButton = new QToolButton ();
		openButton->setDefaultAction (open);
		openButton->setMenu (roMenu);
		openButton->setPopupMode (QToolButton::MenuButtonPopup);
		Toolbar_->addWidget (openButton);

		auto print = new QAction (tr ("Print..."), this);
		print->setProperty ("ActionIcon", "document-print");
		connect (print,
				SIGNAL (triggered ()),
				this,
				SLOT (handlePrint ()));
		Toolbar_->addAction (print);

		auto presentation = new QAction (tr ("Presentation..."), this);
		presentation->setProperty ("ActionIcon", "view-presentation");
		connect (presentation,
				SIGNAL (triggered ()),
				this,
				SLOT (handlePresentation ()));
		Toolbar_->addAction (presentation);

		Toolbar_->addSeparator ();

		auto prev = new QAction (tr ("Previous page"), this);
		prev->setProperty ("ActionIcon", "go-previous-view-page");
		connect (prev,
				SIGNAL (triggered ()),
				this,
				SLOT (handleGoPrev ()));
		Toolbar_->addAction (prev);

		PageNumLabel_ = new QLineEdit ();
		PageNumLabel_->setMaximumWidth (fontMetrics ().width (" 1500 / 1500 "));
		PageNumLabel_->setAlignment (Qt::AlignCenter);
		connect (PageNumLabel_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (navigateNumLabel ()));
		connect (Ui_.PagesView_->verticalScrollBar (),
				SIGNAL (valueChanged (int)),
				this,
				SLOT (updateNumLabel ()));
		connect (Ui_.PagesView_->verticalScrollBar (),
				SIGNAL (valueChanged (int)),
				this,
				SIGNAL (tabRecoverDataChanged ()));
		Toolbar_->addWidget (PageNumLabel_);

		auto next = new QAction (tr ("Next page"), this);
		next->setProperty ("ActionIcon", "go-next-view-page");
		connect (next,
				SIGNAL (triggered ()),
				this,
				SLOT (handleGoNext ()));
		Toolbar_->addAction (next);

		Toolbar_->addSeparator ();

		ScalesBox_ = new QComboBox ();
		ScalesBox_->addItem (tr ("Fit width"));
		ScalesBox_->addItem (tr ("Fit page"));
		std::vector<double> scales = { 0.1, 0.25, 0.33, 0.5, 0.66, 0.8, 1.0, 1.25, 1.5, 2, 3, 4, 5, 7.5, 10 };
		Q_FOREACH (double scale, scales)
			ScalesBox_->addItem (QString::number (scale * 100) + '%', scale);
		ScalesBox_->setCurrentIndex (0);
		connect (ScalesBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleScaleChosen (int)));
		Toolbar_->addWidget (ScalesBox_);

		ZoomOut_ = new QAction (tr ("Zoom out"), this);
		ZoomOut_->setProperty ("ActionIcon", "zoom-out");
		connect (ZoomOut_,
				SIGNAL (triggered ()),
				this,
				SLOT (zoomOut ()));
		Toolbar_->addAction(ZoomOut_);

		ZoomIn_ = new QAction (tr ("Zoom in"), this);
		ZoomIn_->setProperty ("ActionIcon", "zoom-in");
		connect (ZoomIn_,
				SIGNAL (triggered ()),
				this,
				SLOT (zoomIn ()));
		Toolbar_->addAction (ZoomIn_);
		Toolbar_->addSeparator ();

		auto viewGroup = new QActionGroup (this);
		LayOnePage_ = new QAction (tr ("One page"), this);
		LayOnePage_->setProperty ("ActionIcon", "page-simple");
		LayOnePage_->setCheckable (true);
		LayOnePage_->setChecked (true);
		LayOnePage_->setActionGroup (viewGroup);
		connect (LayOnePage_,
				SIGNAL (triggered ()),
				this,
				SLOT (showOnePage ()));
		Toolbar_->addAction (LayOnePage_);

		LayTwoPages_ = new QAction (tr ("Two pages"), this);
		LayTwoPages_->setProperty ("ActionIcon", "page-2sides");
		LayTwoPages_->setCheckable (true);
		LayTwoPages_->setActionGroup (viewGroup);
		connect (LayTwoPages_,
				SIGNAL (triggered ()),
				this,
				SLOT (showTwoPages ()));
		Toolbar_->addAction (LayTwoPages_);

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

	double DocumentTab::GetCurrentScale () const
	{
		if (!CurrentDoc_)
			return 1;

		auto calcRatio = [this] (std::function<double (const QSize&)> dimGetter) -> double
		{
			if (Pages_.isEmpty ())
				return 1.0;
			int pageIdx = GetCurrentPage ();
			if (pageIdx < 0)
				pageIdx = 0;

			double dim = dimGetter (CurrentDoc_->GetPageSize (pageIdx));
			auto size = Ui_.PagesView_->maximumViewportSize ();
			size.rwidth () -= Ui_.PagesView_->verticalScrollBar ()->size ().width ();
			size.rheight () -= Ui_.PagesView_->horizontalScrollBar ()->size ().height ();

			const int margin = 3;
			size.rwidth () -= 2 * margin;
			size.rheight () -= 2 * margin;

			return dimGetter (size) / dim;
		};

		const int idx = ScalesBox_->currentIndex ();
		switch (idx)
		{
		case 0:
		{
			auto ratio = calcRatio ([] (const QSize& size) { return size.width (); });
			if (LayMode_ != LayoutMode::OnePage)
				ratio /= 2;
			return ratio;
		}
		case 1:
		{
			auto wRatio = calcRatio ([] (const QSize& size) { return size.width (); });
			if (LayMode_ != LayoutMode::OnePage)
				wRatio /= 2;
			auto hRatio = calcRatio ([] (const QSize& size) { return size.height (); });
			return std::min (wRatio, hRatio);
		}
		default:
			return ScalesBox_->itemData (idx).toDouble ();
		}
	}

	QPoint DocumentTab::GetViewportCenter () const
	{
		const auto& rect = Ui_.PagesView_->viewport ()->contentsRect ();
		return QPoint (rect.width (), rect.height ()) / 2;
	}

	void DocumentTab::Relayout (double scale)
	{
		RelayoutScheduled_ = false;

		if (!CurrentDoc_)
			return;

		const auto pageWas = GetCurrentPage ();

		for (auto item : Pages_)
			item->SetScale (scale, scale);

		for (int i = 0, pagesCount = Pages_.size (); i < pagesCount; ++i)
		{
			const auto& size = CurrentDoc_->GetPageSize (i) * scale;
			auto page = Pages_ [i];
			switch (LayMode_)
			{
			case LayoutMode::OnePage:
				page->setPos (0, Margin + (size.height () + Margin) * i);
				break;
			case LayoutMode::TwoPages:
				page->setPos ((i % 2) * (Margin / 3 + size.width ()), Margin + (size.height () + Margin) * (i / 2));
				break;
			}
		}

		Scene_.setSceneRect (Scene_.itemsBoundingRect ());
		if (Onload_.Num_ >= 0)
		{
			handleNavigateRequested (QString (), Onload_.Num_, Onload_.X_, Onload_.Y_);
			Onload_.Num_ = -1;
		}
		else
			SetCurrentPage (std::max (pageWas, 0), true);

		updateNumLabel ();
	}

	void DocumentTab::ClearViewActions ()
	{
		Q_FOREACH (auto act, Ui_.PagesView_->actions ())
		{
			Ui_.PagesView_->removeAction (act);
			act->deleteLater ();
		}
	}

	QImage DocumentTab::GetSelectionImg ()
	{
		const auto& bounding = Scene_.selectionArea ().boundingRect ();
		if (bounding.isEmpty ())
			return QImage ();

		QImage image (bounding.size ().toSize (), QImage::Format_ARGB32);
		QPainter painter (&image);
		Scene_.render (&painter, QRectF (), bounding);
		painter.end ();
		return image;
	}

	void DocumentTab::handleNavigateRequested (QString path, int num, double x, double y)
	{
		if (!path.isEmpty ())
		{
			if (QFileInfo (path).isRelative ())
				path = QFileInfo (CurrentDocPath_).dir ().absoluteFilePath (path);

			Onload_ = { num, x, y };

			if (!SetDoc (path))
				Onload_.Num_ = -1;
			return;
		}

		SetCurrentPage (num);

		auto page = Pages_.value (num);
		if (!page)
			return;

		if (x > 0 && y > 0)
		{
			const auto& size = page->boundingRect ().size ();
			const auto& mapped = page->mapToScene (size.width () * x, size.height () * y);
			Ui_.PagesView_->SmoothCenterOn (mapped.x (), mapped.y ());
		}
	}

	void DocumentTab::handlePageSizeChanged (int)
	{
		scheduleRelayout ();
	}

	void DocumentTab::handlePageContentsChanged (int idx)
	{
		auto pageItem = Pages_.at (idx);
		pageItem->UpdatePixmap ();
	}

	void DocumentTab::scheduleRelayout ()
	{
		if (RelayoutScheduled_)
			return;

		QTimer::singleShot (500,
				this,
				SLOT (handleRelayout ()));
		RelayoutScheduled_ = true;
	}

	void DocumentTab::handleRelayout ()
	{
		if (!RelayoutScheduled_)
			return;

		Relayout (GetCurrentScale ());
	}

	void DocumentTab::saveState ()
	{
		if (!SaveStateScheduled_)
			return;

		SaveStateScheduled_ = false;

		if (CurrentDocPath_.isEmpty ())
			return;

		const auto& filename = QFileInfo (CurrentDocPath_).fileName ();
		Core::Instance ().GetDocStateManager ()->SetState (filename,
				{
					GetCurrentPage (),
					LayMode_,
					GetCurrentScale ()
				});
	}

	void DocumentTab::scheduleSaveState ()
	{
		if (SaveStateScheduled_)
			return;

		QTimer::singleShot (1000,
				this,
				SLOT (saveState ()));
		SaveStateScheduled_ = true;
	}

	void DocumentTab::handleRecentOpenAction (QAction *action)
	{
		const auto& path = action->property ("Path").toString ();
		const QFileInfo fi (path);
		if (!fi.exists ())
		{
			QMessageBox::warning (0,
					"LeechCraft",
					tr ("Seems like file %1 doesn't exist anymore.")
						.arg ("<em>" + fi.fileName () + "</em>"));
			return;
		}

		SetDoc (path);
	}

	void DocumentTab::selectFile ()
	{
		const auto& path = QFileDialog::getOpenFileName (this,
					tr ("Select file"),
					QDir::homePath ());
		if (path.isEmpty ())
			return;

		SetDoc (path);
	}

	void DocumentTab::handlePrint ()
	{
		if (!CurrentDoc_)
			return;

		const int numPages = CurrentDoc_->GetNumPages ();

		QPrinter printer;
		QPrintDialog dia (&printer, this);
		dia.setMinMax (1, numPages);
		dia.addEnabledOption (QAbstractPrintDialog::PrintCurrentPage);
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& pageRect = printer.pageRect (QPrinter::Point);
		const auto& pageSize = pageRect.size ();
		const auto resScale = printer.resolution () / 72.0;

		const auto& range = dia.printRange ();
		int start = 0, end = 0;
		switch (range)
		{
		case QAbstractPrintDialog::AllPages:
			start = 0;
			end = numPages;
			break;
		case QAbstractPrintDialog::Selection:
			return;
		case QAbstractPrintDialog::PageRange:
			start = printer.fromPage () - 1;
			end = printer.toPage ();
			break;
		case QAbstractPrintDialog::CurrentPage:
			start = GetCurrentPage ();
			end = start + 1;
			if (start < 0)
				return;
			break;
		}

		QPainter painter (&printer);
		painter.setRenderHint (QPainter::Antialiasing);
		painter.setRenderHint (QPainter::HighQualityAntialiasing);
		painter.setRenderHint (QPainter::SmoothPixmapTransform);
		for (int i = start; i < end; ++i)
		{
			const auto& size = CurrentDoc_->GetPageSize (i);
			const auto scale = std::min (static_cast<double> (pageSize.width ()) / size.width (),
					static_cast<double> (pageSize.height ()) / size.height ());

			const auto& img = CurrentDoc_->RenderPage (i, resScale * scale, resScale * scale);
			painter.drawImage (0, 0, img);

			if (i != end - 1)
				printer.newPage ();
		}
		painter.end ();
	}

	void DocumentTab::handlePresentation ()
	{
		if (!CurrentDoc_)
			return;

		new PresenterWidget (CurrentDoc_);
	}

	void DocumentTab::handleGoPrev ()
	{
		SetCurrentPage (GetCurrentPage () - (LayMode_ == LayoutMode::OnePage ? 1 : 2));

		scheduleSaveState ();
	}

	void DocumentTab::handleGoNext ()
	{
		SetCurrentPage (GetCurrentPage () + (LayMode_ == LayoutMode::OnePage ? 1 : 2));

		scheduleSaveState ();
	}

	void DocumentTab::navigateNumLabel ()
	{
		auto text = PageNumLabel_->text ();
		const int pos = text.indexOf ('/');
		if (pos >= 0)
			text = text.left (pos - 1);

		SetCurrentPage (text.trimmed ().toInt () - 1);

		scheduleSaveState ();
	}

	void DocumentTab::updateNumLabel ()
	{
		if (!CurrentDoc_)
			return;

		const auto& str = QString::number (GetCurrentPage () + 1) +
				" / " +
				QString::number (CurrentDoc_->GetNumPages ());
		PageNumLabel_->setText (str);
	}

	void DocumentTab::zoomOut ()
	{
		const int minIdx = 2;
		auto newIndex = std::max (ScalesBox_->currentIndex () - 1, minIdx);
		ScalesBox_->setCurrentIndex (newIndex);

		ZoomOut_->setEnabled (newIndex > minIdx);
		ZoomIn_->setEnabled (true);
	}

	void DocumentTab::zoomIn ()
	{
		const auto maxIdx = ScalesBox_->count () - 1;
		auto newIndex = std::min (ScalesBox_->currentIndex () + 1, maxIdx);
		ScalesBox_->setCurrentIndex (newIndex);

		ZoomOut_->setEnabled (true);
		ZoomIn_->setEnabled (newIndex < maxIdx);
	}

	void DocumentTab::showOnePage ()
	{
		if (LayMode_ == LayoutMode::OnePage)
			return;

		LayMode_ = LayoutMode::OnePage;
		Relayout (GetCurrentScale ());

		emit tabRecoverDataChanged ();

		scheduleSaveState ();
	}

	void DocumentTab::showTwoPages ()
	{
		if (LayMode_ == LayoutMode::TwoPages)
			return;

		LayMode_ = LayoutMode::TwoPages;
		Relayout (GetCurrentScale ());

		emit tabRecoverDataChanged ();

		scheduleSaveState ();
	}

	void DocumentTab::syncUIToLayMode ()
	{
		auto action = LayMode_ == LayoutMode::OnePage ? LayOnePage_ : LayTwoPages_;
		action->setChecked (true);
	}

	void DocumentTab::setMoveMode (bool enable)
	{
		if (!enable)
			return;

		MouseMode_ = MouseMode::Move;
		Ui_.PagesView_->SetShowReleaseMenu (false);

		ClearViewActions ();
		Ui_.PagesView_->setDragMode (QGraphicsView::ScrollHandDrag);
	}

	void DocumentTab::setSelectionMode (bool enable)
	{
		if (!enable)
			return;

		MouseMode_ = MouseMode::Select;
		Ui_.PagesView_->SetShowReleaseMenu (true);

		ClearViewActions ();
		Ui_.PagesView_->setDragMode (QGraphicsView::RubberBandDrag);

		auto copyAsImage = new QAction (tr ("Copy selection as image"), this);
		connect (copyAsImage,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCopyAsImage ()));
		Ui_.PagesView_->addAction (copyAsImage);

		auto saveAsImage = new QAction (tr ("Save selection as image..."), this);
		connect (saveAsImage,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSaveAsImage ()));
		Ui_.PagesView_->addAction (saveAsImage);

		if (qobject_cast<IHaveTextContent*> (CurrentDoc_->GetObject ()))
		{
			Ui_.PagesView_->addAction (Util::CreateSeparator (Ui_.PagesView_));

			auto copyAsText = new QAction (tr ("Copy selection as text"), this);
			connect (copyAsText,
					SIGNAL (triggered ()),
					this,
					SLOT (handleCopyAsText ()));
			Ui_.PagesView_->addAction (copyAsText);
		}
	}

	void DocumentTab::handleCopyAsImage ()
	{
		QApplication::clipboard ()->setImage (GetSelectionImg ());
	}

	void DocumentTab::handleSaveAsImage ()
	{
		const auto& image = GetSelectionImg ();
		if (image.isNull ())
			return;

		const auto& previous = XmlSettingsManager::Instance ()
				.Property ("SelectionImageSavePath", QDir::homePath ()).toString ();
		const auto& filename = QFileDialog::getSaveFileName (this,
				tr ("Save selection as"),
				previous,
				tr ("PNG images (*.png)"));
		if (filename.isEmpty ())
			return;

		const QFileInfo saveFI (filename);
		XmlSettingsManager::Instance ().setProperty ("SelectionImageSavePath",
				saveFI.absoluteFilePath ());
		const auto& userSuffix = saveFI.suffix ().toLatin1 ();
		const auto& supported = QImageWriter::supportedImageFormats ();
		const auto suffix = supported.contains (userSuffix) ?
				userSuffix :
				QByteArray ("PNG");
		image.save (filename, suffix, 100);
	}

	void DocumentTab::handleCopyAsText ()
	{
		auto ihtc = qobject_cast<IHaveTextContent*> (CurrentDoc_->GetObject ());
		if (!ihtc)
			return;

		const auto& selectionBound = Scene_.selectionArea ().boundingRect ();

		auto bounding = Ui_.PagesView_->mapFromScene (selectionBound).boundingRect ();
		if (bounding.isEmpty () ||
				bounding.width () < 4 ||
				bounding.height () < 4)
		{
			qWarning () << Q_FUNC_INFO
					<< "selection area is empty";
			return;
		}

		auto item = Ui_.PagesView_->itemAt (bounding.topLeft ());
		auto pageItem = dynamic_cast<PageGraphicsItem*> (item);
		if (!pageItem)
		{
			qWarning () << Q_FUNC_INFO
					<< "page item is null for"
					<< bounding.topLeft ();
			return;
		}

		bounding = item->mapFromScene (selectionBound).boundingRect ().toRect ();

		const auto scale = GetCurrentScale ();
		bounding.moveTopLeft (bounding.topLeft () / scale);
		bounding.setSize (bounding.size () / scale);

		const auto& text = ihtc->GetTextContent (pageItem->GetPageNum (), bounding);
		QApplication::clipboard ()->setText (text);
	}

	void DocumentTab::showDocInfo ()
	{
		if (!CurrentDoc_)
			return;

		auto dia = new DocInfoDialog (CurrentDocPath_, CurrentDoc_->GetDocumentInfo (), this);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}

	void DocumentTab::delayedCenterOn (const QPoint& point)
	{
		Ui_.PagesView_->SmoothCenterOn (point.x (), point.y ());
	}

	void DocumentTab::handleScaleChosen (int)
	{
		Relayout (GetCurrentScale ());

		scheduleSaveState ();

		emit tabRecoverDataChanged ();
	}
}
}
