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
#include <QtDebug>

namespace LeechCraft
{
namespace Monocle
{
namespace Postrus
{
	Document::Document (const QString& path, QObject *parent)
	: QObject (parent)
	, SD_ (spectre_document_new ())
	{
		spectre_document_load (SD_, path.toUtf8 ().constData ());
	}

	Document::~Document ()
	{
		spectre_document_free (SD_);
	}

	QObject* Document::GetObject ()
	{
		return this;
	}

	bool Document::IsValid () const
	{
		return spectre_document_status (SD_) != SPECTRE_STATUS_LOAD_ERROR;
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		DocumentInfo info;
		if (const char *title = spectre_document_get_title (SD_))
			info.Title_ = QString::fromUtf8 (title);
		if (const char *author = spectre_document_get_creator (SD_))
			info.Author_ = QString::fromUtf8 (author);
		return info;
	}

	int Document::GetNumPages () const
	{
		return spectre_document_get_n_pages (SD_);
	}

	namespace
	{
		QSize GetSpectrePageSize (SpectrePage *page)
		{
			QSize result;
			spectre_page_get_size (page, &result.rwidth (), &result.rheight ());
			return result;
		}
	}

	QSize Document::GetPageSize (int index) const
	{
		auto page = spectre_document_get_page (SD_, index);
		const auto& result = GetSpectrePageSize (page);
		spectre_page_free (page);
		return result;
	}

	QImage Document::RenderPage (int index, double xRes, double yRes)
	{
		auto page = spectre_document_get_page (SD_, index);

		auto rc = spectre_render_context_new ();
		auto size = GetPageSize (index);
		spectre_render_context_set_scale (rc, xRes, yRes);
		size.rwidth () *= xRes;
		size.rheight () *= yRes;

		unsigned char *data = 0;
		int rowLength = 0;
		spectre_page_render (page, rc, &data, &rowLength);
		spectre_render_context_free (rc);
		spectre_page_free (page);

		const QImage& img = rowLength == size.width () * 4 ?
				QImage (data, size.width (), size.height (), QImage::Format_RGB32) :
				QImage (data, rowLength / 4, size.height (), QImage::Format_RGB32)
					.copy (0, 0, size.width (), size.height ());
		free (data);
		return img;
	}

	QList<ILink_ptr> Document::GetPageLinks (int)
	{
		return QList<ILink_ptr> ();
	}
}
}
}
