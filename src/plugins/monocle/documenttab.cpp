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
		std::vector<double> scales = { 0.1, 0.25, 0.33, 0.5, 0.66, 0.8, 1.0, 1.25, 1.5, 2 };
		Q_FOREACH (double scale, scales)
			ScalesBox_->addItem (QString::number (scale * 100) + '%', scale);
		ScalesBox_->setCurrentIndex (std::distance (scales.begin (),
					std::find (scales.begin (), scales.end (), 1)));
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
		return ScalesBox_->itemData (ScalesBox_->currentIndex ()).toDouble ();
	}

	int DocumentTab::GetCurrentPage () const
	{
		const auto& rect = Ui_.PagesView_->viewport ()->contentsRect ();
		auto item = Ui_.PagesView_->itemAt (QPoint (rect.width (), rect.height ()) / 2);
		if (!item)
			item = Ui_.PagesView_->itemAt (QPoint (rect.width () - 2 * Margin, rect.height () - 2 * Margin) / 2);
		return Pages_.indexOf (static_cast<PageGraphicsItem*> (item));
	}

	void DocumentTab::SetCurrentPage (int pos)
	{
		if (pos >= 0 && pos < Pages_.size ())
			Ui_.PagesView_->ensureVisible (Pages_.at (pos), Margin, Margin);
	}

	void DocumentTab::Relayout (double scale)
	{
		if (!CurrentDoc_)
			return;

		Q_FOREACH (auto item, Pages_)
			item->SetScale (scale, scale);

		for (int i = 0, size = Pages_.size (); i < size; ++i)
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
		updateNumLabel ();
	}

	void DocumentTab::selectFile ()
	{
		const auto& path = QFileDialog::getOpenFileName (this,
					tr ("Select file"),
					QDir::homePath ());
		if (path.isEmpty ())
			return;

		Scene_.clear ();
		Pages_.clear ();

		CurrentDoc_ = Core::Instance ().LoadDocument (path);
		if (!CurrentDoc_)
		{
			emit changeTabName (this, TC_.VisibleName_);
			return;
		}

		const auto& title = QFileInfo (path).fileName ();
		emit changeTabName (this, title);

		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem (CurrentDoc_, i);
			Scene_.addItem (item);
			Pages_ << item;
		}

		Ui_.PagesView_->ensureVisible (Pages_.value (0), Margin, Margin);

		Relayout (1);

		updateNumLabel ();
	}

	void DocumentTab::handleGoPrev ()
	{
		SetCurrentPage (GetCurrentPage () - 1);
	}

	void DocumentTab::handleGoNext ()
	{
		SetCurrentPage (GetCurrentPage () + 1);
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
