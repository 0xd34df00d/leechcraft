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
#include "seen.h"
#include "docmanager.h"

namespace LeechCraft
{
namespace Monocle
{
namespace Seen
{
	Document::Document (const QString& file, ddjvu_context_t *ctx, DocManager *mgr)
	: Context_ (ctx)
	, Doc_ (ddjvu_document_create_by_filename_utf8 (Context_, file.toUtf8 ().constData (), 1))
	, DocMgr_ (mgr)
	{
		if (ddjvu_document_get_type (Doc_) != DDJVU_DOCTYPE_UNKNOWN)
			UpdateDocInfo ();
	}

	Document::~Document ()
	{
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

	QImage Document::RenderPage (int pageNum, double xRes, double yRes)
	{
		return QImage ().scaled (GetPageSize (pageNum));
	}

	QList<ILink_ptr> Document::GetPageLinks (int page)
	{
		return QList<ILink_ptr> ();
	}

	ddjvu_document_t* Document::GetNativeDoc () const
	{
		return Doc_;
	}

	void Document::UpdateDocInfo ()
	{
		TryUpdateSizes ();
	}

	void Document::UpdatePageInfo (ddjvu_page_t *page)
	{
		TryUpdateSizes ();
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
