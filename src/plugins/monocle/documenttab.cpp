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
#include <array>
#include <QToolBar>
#include <QComboBox>
#include <QFileDialog>
#include "core.h"
#include "pagegraphicsitem.h"

namespace LeechCraft
{
namespace Monocle
{
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

		ScalesBox_ = new QComboBox;
		std::vector<double> scales = { 0.1, 0.25, 0.33, 0.5, 0.66, 0.8, 1.0, 1.25, 1.5, 2 };
		Q_FOREACH (double scale, scales)
			ScalesBox_->addItem (QString::number (scale * 100) + '%', scale);
		connect (ScalesBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleScaleChosen (int)));
		Toolbar_->addWidget (ScalesBox_);
	}

	void DocumentTab::Relayout (double scale)
	{
		if (!CurrentDoc_)
			return;

		for (int i = 0, size = Pages_.size (); i < size; ++i)
		{
			const auto& size = CurrentDoc_->GetPageSize (i) * scale;
			Pages_ [i]->setPos (0, (size.height () + 10) * i);
		}
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
		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem (CurrentDoc_, i);
			Scene_.addItem (item);
			Pages_ << item;
		}

		Ui_.PagesView_->ensureVisible (Pages_.value (0), 0, 0);

		Relayout (1);
	}

	void DocumentTab::handleScaleChosen (int index)
	{
		double scale = ScalesBox_->itemData (index).toDouble ();

		Q_FOREACH (auto item, Pages_)
			item->SetScale (scale, scale);

		Relayout (scale);
	}
}
}
