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

#include "document.h"
#include "seen.h"
#include "docmanager.h"

namespace LeechCraft
{
namespace Monocle
{
namespace Seen
{
	namespace
	{
		static unsigned int FormatMask [4] = { 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };
	}

	Document::Document (const QString& file, ddjvu_context_t *ctx, DocManager *mgr)
	: Context_ (ctx)
	, Doc_ (ddjvu_document_create_by_filename_utf8 (Context_, file.toUtf8 ().constData (), 1))
	, RenderFormat_ (ddjvu_format_create (DDJVU_FORMAT_RGBMASK32, 4, FormatMask))
	, DocMgr_ (mgr)
	, DocURL_ (QUrl::fromLocalFile (file))
	{
		ddjvu_format_set_row_order (RenderFormat_, 1);
		ddjvu_format_set_y_direction (RenderFormat_, 1);

		if (ddjvu_document_get_type (Doc_) != DDJVU_DOCTYPE_UNKNOWN)
			UpdateDocInfo ();
	}

	Document::~Document ()
	{
		ddjvu_format_release (RenderFormat_);
		DocMgr_->Unregister (Doc_);
		ddjvu_document_release (Doc_);
	}

	QObject* Document::GetObject ()
	{
		return this;
	}

	bool Document::IsValid () const
	{
		return Doc_;
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		return DocumentInfo ();
	}

	int Document::GetNumPages () const
	{
		return ddjvu_document_get_pagenum (Doc_);
	}

	QSize Document::GetPageSize (int pageNum) const
	{
		return Sizes_.value (pageNum);
	}

	QImage Document::RenderPage (int pageNum, double xScale, double yScale)
	{
		ddjvu_page_t *page = 0;
		if (PendingRenders_.contains (pageNum))
			page = PendingRenders_ [pageNum];
		else
		{
			page = ddjvu_page_create_by_pageno (Doc_, pageNum);
			PendingRenders_ [pageNum] = page;
			PendingRendersNums_ [page] = pageNum;
		}

		const auto& size = Sizes_.value (pageNum);
		ddjvu_rect_s rect =
		{
			0,
			0,
			static_cast<unsigned int> (size.width ()),
			static_cast<unsigned int> (size.height ())
		};

		QImage img (size, QImage::Format_RGB32);

		auto res = ddjvu_page_render (page,
				DDJVU_RENDER_COLOR,
				&rect,
				&rect,
				RenderFormat_,
				img.bytesPerLine (),
				reinterpret_cast<char*> (img.bits ()));
		qDebug () << Q_FUNC_INFO << pageNum << res;
		if (res)
		{
			PendingRenders_.remove (pageNum);
			PendingRendersNums_.remove (page);
			ddjvu_page_release (page);
		}

		return img.scaled (img.width () * xScale, img.height () * yScale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	QList<ILink_ptr> Document::GetPageLinks (int)
	{
		return QList<ILink_ptr> ();
	}

	QUrl Document::GetDocURL () const
	{
		return DocURL_;
	}

	ddjvu_document_t* Document::GetNativeDoc () const
	{
		return Doc_;
	}

	void Document::UpdateDocInfo ()
	{
		TryUpdateSizes ();
	}

	void Document::UpdatePageInfo (ddjvu_page_t*)
	{
		TryUpdateSizes ();
	}

	void Document::RedrawPage (ddjvu_page_t *page)
	{
		auto num = PendingRendersNums_ [page];
		emit pageContentsChanged (num);
	}

	void Document::TryUpdateSizes ()
	{
		const int numPages = GetNumPages ();
		for (int i = 0; i < numPages; ++i)
			if (!Sizes_.contains (i))
				TryGetPageInfo (i);
	}

	void Document::TryGetPageInfo (int pageNum)
	{
		ddjvu_pageinfo_t info;
		auto r = ddjvu_document_get_pageinfo (Doc_, pageNum, &info);
		if (r != DDJVU_JOB_OK)
			return;

		Sizes_ [pageNum] = QSize (info.width, info.height);
		emit pageSizeChanged (pageNum);
	}
}
}
}
