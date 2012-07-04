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

#include "documentadapter.h"
#include <QTextDocument>
#include <QPainter>
#include <QtDebug>

namespace LeechCraft
{
namespace Monocle
{
namespace FXB
{
	DocumentAdapter::DocumentAdapter (QTextDocument *doc)
	{
		SetDocument (doc);
	}

	bool DocumentAdapter::IsValid () const
	{
		return static_cast<bool> (Doc_);
	}

	int DocumentAdapter::GetNumPages () const
	{
		return Doc_->pageCount ();
	}

	QSize DocumentAdapter::GetPageSize (int) const
	{
		auto size = Doc_->pageSize ();
		size.setWidth (std::ceil (size.width ()));
		return size.toSize ();
	}

	QImage DocumentAdapter::RenderPage (int page, double xScale, double yScale)
	{
		const auto& size = Doc_->pageSize ();

		auto imgSize = size.toSize ();
		imgSize.rwidth () *= xScale;
		imgSize.rheight () *= yScale;
		QImage image (imgSize, QImage::Format_ARGB32);
		image.fill (Qt::white);

		QRectF rect (QPointF (0, 0), size);
		rect.moveTop (rect.height () * page);

		QPainter painter;
		painter.begin (&image);
		painter.scale (xScale, yScale);
		painter.translate (0, rect.height () * (-page));
		Doc_->drawContents (&painter, rect);
		painter.end ();

		return image;
	}

	QList<ILink_ptr> DocumentAdapter::GetPageLinks (int)
	{
		return QList<ILink_ptr> ();
	}

	void DocumentAdapter::SetDocument (QTextDocument *doc)
	{
		Doc_.reset (doc);
	}
}
}
}
