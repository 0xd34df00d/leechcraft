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

#include "pagegraphicsitem.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Monocle
{
	PageGraphicsItem::PageGraphicsItem (IDocument_ptr doc, int page, QGraphicsItem *parent)
	: QGraphicsPixmapItem (parent)
	, Doc_ (doc)
	, PageNum_ (page)
	, XScale_ (1)
	, YScale_ (1)
	, Invalid_ (true)
	{
		setPixmap (QPixmap (Doc_->GetPageSize (page)));
	}

	void PageGraphicsItem::SetScale (double xs, double ys)
	{
		XScale_ = xs;
		YScale_ = ys;

		Invalid_ = true;

		update ();
	}

	void PageGraphicsItem::paint (QPainter *painter,
			const QStyleOptionGraphicsItem *option, QWidget *w)
	{
		if (Invalid_)
		{
			const auto& img = Doc_->RenderPage (PageNum_, XScale_, YScale_);
			setPixmap (QPixmap::fromImage (img));
			Invalid_ = false;
		}
		QGraphicsPixmapItem::paint (painter, option, w);
	}
}
}
