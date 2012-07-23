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

#include "document.h"
#include <memory>
#include <cstring>
#include <QPainter>
#include <QtDebug>

namespace LeechCraft
{
namespace Monocle
{
namespace Mu
{
	namespace
	{
		std::shared_ptr<pdf_page> WrapPage (pdf_page *pg, pdf_document *doc)
		{
			return std::shared_ptr<pdf_page> (pg,
					[doc] (pdf_page *page)
					{
						if (page)
							pdf_free_page (doc, page);
					});
		}
	}

	Document::Document (const QString& filename, fz_context *ctx)
	: MuCtx_ (ctx)
	, MuDoc_ (pdf_open_document (ctx, filename.toUtf8 ().constData ()))
	{
	}

	Document::~Document ()
	{
		pdf_close_document (MuDoc_);
	}

	QObject* Document::GetObject ()
	{
		return this;
	}

	bool Document::IsValid () const
	{
		return MuDoc_;
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		return DocumentInfo ();
	}

	int Document::GetNumPages () const
	{
		return pdf_count_pages (MuDoc_);
	}

	QSize Document::GetPageSize (int num) const
	{
		auto page = WrapPage (pdf_load_page (MuDoc_, num), MuDoc_);
		if (!page)
			return QSize ();

		const auto& rect = pdf_bound_page (MuDoc_, page.get ());
		return QSize (rect.x1 - rect.x0,
				rect.y1 - rect.y0);
	}

	QImage Document::RenderPage (int num, double xRes, double yRes)
	{
		auto page = WrapPage (pdf_load_page (MuDoc_, num), MuDoc_);
		if (!page)
			return QImage ();

		const auto& rect = pdf_bound_page (MuDoc_, page.get ());

		auto px = fz_new_pixmap (MuCtx_, fz_device_bgr, xRes * (rect.x1 - rect.x0), yRes * (rect.y1 - rect.y0));
		fz_clear_pixmap (MuCtx_, px);
		auto dev = fz_new_draw_device (MuCtx_, px);
		pdf_run_page (MuDoc_, page.get (), dev, fz_scale (xRes, yRes), NULL);
		fz_free_device (dev);

		const int pxWidth = fz_pixmap_width (MuCtx_, px);
		const int pxHeight = fz_pixmap_height (MuCtx_, px);

		auto samples = fz_pixmap_samples (MuCtx_, px);

		QImage temp (samples, pxWidth, pxHeight, QImage::Format_ARGB32);
		QImage img (QSize (pxWidth, pxHeight), QImage::Format_ARGB32);

		for (int y = 0; y < pxHeight; ++y)
		{
			auto target = reinterpret_cast<QRgb*> (img.scanLine (y));
			const auto source = reinterpret_cast<QRgb*> (temp.scanLine (y));
			std::memcpy (target, source, sizeof (source [0]) * pxWidth);
		}
		fz_drop_pixmap (MuCtx_, px);

		temp = QImage (QSize (pxWidth, pxHeight), QImage::Format_ARGB32);

		QPainter p;
		p.begin (&temp);
		p.fillRect (QRect (QPoint (0, 0), temp.size ()), Qt::white);
		p.drawImage (0, 0, img);
		p.end ();
		return temp;
	}

	QList<ILink_ptr> Document::GetPageLinks (int)
	{
		return QList<ILink_ptr> ();
	}
}
}
}
