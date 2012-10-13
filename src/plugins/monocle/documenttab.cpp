/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <interfaces/imwproxy.h>
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
	, DockTOC_ (0)
	, TOCWidget_ (new TOCWidget ())
	, LayMode_ (LayoutMode::OnePage)
	, MouseMode_ (MouseMode::Move)
	, RelayoutScheduled_ (true)
	, Onload_ ({ -1, 0, 0 })
	{
		Ui_.setupUi (this);
		Ui_.PagesView_->setScene (&Scene_);
		Ui_.PagesView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		SetupToolbar ();

		new FileWatcher (this);

		auto mw = Core::Instance ().GetProxy ()->GetMWProxy ();

		DockTOC_ = new QDockWidget (tr ("Table of contents"));
		DockTOC_->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
		DockTOC_->setWidget (TOCWidget_);

		const auto& icon = Core::Instance ().GetProxy ()->GetIcon ("view-table-of-contents-ltr");
		DockTOC_->setWindowIcon (icon);
		DockTOC_->toggleViewAction ()->setIcon (icon);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (DockTOC_->toggleViewAction ());

		mw->AddDockWidget (Qt::RightDockWidgetArea, DockTOC_);
		mw->AssociateDockWidget (DockTOC_, this);
		mw->ToggleViewActionVisiblity (DockTOC_, false);
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
		delete DockTOC_;
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

		const auto& rectSize = Ui_.PagesView_->viewport ()->contentsRect ().size () / 2;
		const auto& pos = Ui_.PagesView_->mapToScene (QPoint (rectSize.width (), rectSize.height ()));

		SetDoc (doc);

		if (Scene_.itemsBoundingRect ().contains (pos))
			Ui_.PagesView_->centerOn (pos);
	}

	bool DocumentTab::SetDoc (const QString& path)
	{
		auto document = Core::Instance ().LoadDocument (path);
		if (!document)
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
		Ui_.PagesView_->ensureVisible (Pages_.value (0), Margin, Margin);
		Relayout (GetCurrentScale ());

		updateNumLabel ();

		TOCEntryLevel_t topLevel;
		if (auto toc = qobject_cast<IHaveTOC*> (CurrentDoc_->GetObject ()))
			topLevel = toc->GetTOC ();
		TOCWidget_->SetTOC (topLevel);
		DockTOC_->setEnabled (!topLevel.isEmpty ());
		if (DockTOC_->toggleViewAction ()->isChecked () == topLevel.isEmpty ())
			DockTOC_->toggleViewAction ()->trigger ();

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
		std::vector<double> scales = { 0.1, 0.25, 0.33, 0.5, 0.66, 0.8, 1.0, 1.25, 1.5, 2 };
		Q_FOREACH (double scale, scales)
			ScalesBox_->addItem (QString::number (scale * 100) + '%', scale);
		ScalesBox_->setCurrentIndex (0);
		connect (ScalesBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleScaleChosen (int)));
		Toolbar_->addWidget (ScalesBox_);

		auto viewGroup = new QActionGroup (this);
		auto onePage = new QAction (tr ("One page"), this);
		onePage->setProperty ("ActionIcon", "page-simple");
		onePage->setCheckable (true);
		onePage->setChecked (true);
		onePage->setActionGroup (viewGroup);
		connect (onePage,
				SIGNAL (triggered ()),
				this,
				SLOT (showOnePage ()));
		Toolbar_->addAction (onePage);

		auto twoPages = new QAction (tr ("Two pages"), this);
		twoPages->setProperty ("ActionIcon", "page-2sides");
		twoPages->setCheckable (true);
		twoPages->setActionGroup (viewGroup);
		connect (twoPages,
				SIGNAL (triggered ()),
				this,
				SLOT (showTwoPages ()));
		Toolbar_->addAction (twoPages);

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

	void DocumentTab::SetCurrentPage (int idx)
	{
		if (idx < 0 || idx >= Pages_.size ())
			return;

		auto page = Pages_.at (idx);
		const auto& rect = page->boundingRect ();
		const auto& pos = page->scenePos ();
		int xCenter = pos.x () + rect.width () / 2;
		int yCenter = pos.y () + Ui_.PagesView_->viewport ()->contentsRect ().height () / 2;
		Ui_.PagesView_->centerOn (xCenter, yCenter);
	}

	void DocumentTab::Relayout (double scale)
	{
		RelayoutScheduled_ = false;

		if (!CurrentDoc_)
			return;

		Q_FOREACH (auto item, Pages_)
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
			SetCurrentPage (std::max (GetCurrentPage (), 0));
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
			Ui_.PagesView_->centerOn (mapped.x (), mapped.y ());
		}
	}

	void DocumentTab::handlePageSizeChanged (int)
	{
		if (RelayoutScheduled_)
			return;

		QTimer::singleShot (500,
				this,
				SLOT (handleRelayout ()));
		RelayoutScheduled_ = true;
	}

	void DocumentTab::handlePageContentsChanged (int idx)
	{
		auto pageItem = Pages_.at (idx);
		pageItem->UpdatePixmap ();
	}

	void DocumentTab::handleRelayout ()
	{
		if (!RelayoutScheduled_)
			return;

		Relayout (GetCurrentScale ());
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
	}

	void DocumentTab::handleGoNext ()
	{
		SetCurrentPage (GetCurrentPage () + (LayMode_ == LayoutMode::OnePage ? 1 : 2));
	}

	void DocumentTab::navigateNumLabel ()
	{
		auto text = PageNumLabel_->text ();
		const int pos = text.indexOf ('/');
		if (pos >= 0)
			text = text.left (pos - 1);

		SetCurrentPage (text.trimmed ().toInt () - 1);
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

	void DocumentTab::showOnePage ()
	{
		LayMode_ = LayoutMode::OnePage;
		Relayout (GetCurrentScale ());

		emit tabRecoverDataChanged ();
	}

	void DocumentTab::showTwoPages ()
	{
		LayMode_ = LayoutMode::TwoPages;
		Relayout (GetCurrentScale ());

		emit tabRecoverDataChanged ();
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

		if (qobject_cast<IHaveTextContent*> (CurrentDoc_->GetObject ()))
		{
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
		const auto& bounding = Scene_.selectionArea ().boundingRect ();
		if (bounding.isEmpty ())
			return;

		QImage image (bounding.size ().toSize (), QImage::Format_ARGB32);
		QPainter painter (&image);
		Scene_.render (&painter, QRectF (), bounding);
		painter.end ();

		QApplication::clipboard ()->setImage (image);

		Ui_.PagesView_->SetShowReleaseMenu (false);
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

	void DocumentTab::delayedCenterOn (const QPoint& point)
	{
		Ui_.PagesView_->centerOn (point);
	}

	void DocumentTab::handleScaleChosen (int)
	{
		Relayout (GetCurrentScale ());

		emit tabRecoverDataChanged ();
	}
}
}
