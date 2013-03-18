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

#include "thumbswidget.h"
#include "pageslayoutmanager.h"
#include "pagegraphicsitem.h"
#include "common.h"

namespace LeechCraft
{
namespace Monocle
{
	ThumbsWidget::ThumbsWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.ThumbsView_->setScene (&Scene_);

		LayoutMgr_ = new PagesLayoutManager (Ui_.ThumbsView_, this);
		LayoutMgr_->SetScaleMode (ScaleMode::FitWidth);
	}

	void ThumbsWidget::HandleDoc (IDocument_ptr doc)
	{
		Scene_.clear ();
		CurrentDoc_ = doc;

		if (!doc)
			return;

		QList<PageGraphicsItem*> pages;
		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem (CurrentDoc_, i);
			Scene_.addItem (item);
			pages << item;
		}

		LayoutMgr_->HandleDoc (CurrentDoc_, pages);
		LayoutMgr_->Relayout ();
	}
}
}

