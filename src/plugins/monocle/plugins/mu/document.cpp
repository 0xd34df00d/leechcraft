/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "document.h"
#include <memory>
#include <cstring>
#include <QPainter>
#include <QtDebug>

namespace LC
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

	Document::Document (const QString& filename, fz_context *ctx, QObject *plugin)
	: MuCtx_ (ctx)
	, MuDoc_ (pdf_open_document (ctx, filename.toUtf8 ().constData ()))
	, URL_ (QUrl::fromLocalFile (filename))
	, Plugin_ (plugin)
	{
	}

	Document::~Document ()
	{
		pdf_close_document (MuDoc_);
	}

	QObject* Document::GetBackendPlugin () const
	{
		return Plugin_;
	}

	QObject* Document::GetQObject ()
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

#if MUPDF_VERSION < 0x0102
		const auto& rect = pdf_bound_page (MuDoc_, page.get ());
#else
		fz_rect rect;
		pdf_bound_page (MuDoc_, page.get (), &rect);
#endif
		return QSize (rect.x1 - rect.x0,
				rect.y1 - rect.y0);
	}

	QImage Document::RenderPage (int num, double xRes, double yRes)
	{
		auto page = WrapPage (pdf_load_page (MuDoc_, num), MuDoc_);
		if (!page)
			return QImage ();

#if MUPDF_VERSION < 0x0102
		const auto& rect = pdf_bound_page (MuDoc_, page.get ());
#else
		fz_rect rect;
		pdf_bound_page (MuDoc_, page.get (), &rect);
#endif

		auto px = fz_new_pixmap (MuCtx_, fz_device_bgr, xRes * (rect.x1 - rect.x0), yRes * (rect.y1 - rect.y0));
		fz_clear_pixmap (MuCtx_, px);
		auto dev = fz_new_draw_device (MuCtx_, px);
#if MUPDF_VERSION < 0x0102
		pdf_run_page (MuDoc_, page.get (), dev, fz_scale (xRes, yRes), NULL);
#else
		fz_matrix matrix;
		pdf_run_page (MuDoc_, page.get (), dev, fz_scale (&matrix, xRes, yRes), NULL);
#endif
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

	QUrl Document::GetDocURL () const
	{
		return URL_;
	}
}
}
}
