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

namespace LeechCraft
{
namespace Monocle
{
namespace Seen
{
	Document::Document (const QString& file, ddjvu_context_t *ctx)
	: Context_ (ctx)
	, Doc_ (ddjvu_document_create_by_filename_utf8 (Context_, file.toUtf8 ().constData (), 1))
	{
	}

	Document::~Document ()
	{
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
		return QSize ();
	}

	QImage Document::RenderPage (int , double xRes, double yRes)
	{
		return QImage ();
	}

	QList<ILink_ptr> Document::GetPageLinks (int page)
	{
		return QList<ILink_ptr> ();
	}
}
}
}
