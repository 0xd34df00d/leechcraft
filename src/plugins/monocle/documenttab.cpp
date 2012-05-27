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
#include <QtDebug>
#include "core.h"
#include "pagegraphicsitem.h"

namespace LeechCraft
{
namespace Monocle
{
	const int Margin = 10;

	DocumentTab::DocumentTab (const TabClassInfo& tc, QObject* parent)
	: TC_ (tc)
	, ParentPlugin_ (parent)
	, Toolbar_ (new QToolBar ("Monocle"))
	, LayMode_ (LayoutMode::OnePage)
	{
		Ui_.setupUi (this);
		Ui_.PagesView_->setScene (&Scene_);
		Ui_.PagesView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		SetupToolbar ();
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
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* DocumentTab::GetToolBar () const
	{
		return Toolbar_;
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
		Toolbar_->addAction (open);

		auto print = new QAction (tr ("Print..."), this);
		print->setProperty ("ActionIcon", "document-print");
		connect (print,
				SIGNAL (triggered ()),
				this,
				SLOT (handlePrint ()));
		Toolbar_->addAction (print);

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
	}

	double DocumentTab::GetCurrentScale () const
	{
		if (!CurrentDoc_)
			return 1;

		auto calcRatio = [this] (std::function<double (const QSize&)> dimGetter)
		{
			const int pageIdx = GetCurrentPage ();
			if (pageIdx < 0)
				return 1.0;

			double dim = dimGetter (CurrentDoc_->GetPageSize (pageIdx));
			return dimGetter (Ui_.PagesView_->viewport ()->contentsRect ().size ()) / dim;
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

		Scene_.clear ();
		Pages_.clear ();

		CurrentDoc_ = document;
		const auto& title = QFileInfo (path).fileName ();
		emit changeTabName (this, title);

		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem (CurrentDoc_, i);
			Scene_.addItem (item);
			Pages_ << item;
		}
		Ui_.PagesView_->ensureVisible (Pages_.value (0), Margin, Margin);
		Relayout (GetCurrentScale ());

		updateNumLabel ();

		connect (CurrentDoc_->GetObject (),
				SIGNAL (navigateRequested (QString, int, double, double)),
				this,
				SLOT (handleNavigateRequested (QString, int, double, double)),
				Qt::UniqueConnection);
		return true;
	}

	int DocumentTab::GetCurrentPage () const
	{
		const auto& rect = Ui_.PagesView_->viewport ()->contentsRect ();
		auto item = Ui_.PagesView_->itemAt (QPoint (rect.width () - 1, rect.height () - 1) / 2);
		if (!item)
			item = Ui_.PagesView_->itemAt (QPoint (rect.width () - 2 * Margin, rect.height () - 2 * Margin) / 2);
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
		SetCurrentPage (std::max (GetCurrentPage (), 0));
		updateNumLabel ();
	}

	void DocumentTab::handleNavigateRequested (const QString& path, int num, double x, double y)
	{
		if (!path.isEmpty ())
			if (!SetDoc (path))
				return;

		SetCurrentPage (num);

		auto page = Pages_.value (num);
		if (!page)
			return;

		if (x > 0 && y > 0)
		{
			const auto& size = page->boundingRect ().size ();
			const auto& mapped = page->mapToScene (size.width () * x, size.height () * y);
			Ui_.PagesView_->ensureVisible (mapped.x (), mapped.y (), 0, 0);
		}
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
	}

	void DocumentTab::showTwoPages ()
	{
		LayMode_ = LayoutMode::TwoPages;
		Relayout (GetCurrentScale ());
	}

	void DocumentTab::handleScaleChosen (int)
	{
		Relayout (GetCurrentScale ());
	}
}
}
