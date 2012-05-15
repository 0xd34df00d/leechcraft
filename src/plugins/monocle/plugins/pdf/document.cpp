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
#include "links.h"
#include <poppler-qt4.h>

namespace LeechCraft
{
namespace Monocle
{
namespace PDF
{
	Document::Document (const QString& path, QObject *parent)
	: QObject (parent)
	, PDocument_ (Poppler::Document::load (path))
	{
		if (!PDocument_)
			return;

		PDocument_->setRenderHint (Poppler::Document::Antialiasing);
		PDocument_->setRenderHint (Poppler::Document::TextAntialiasing);
		PDocument_->setRenderHint (Poppler::Document::TextHinting);
	}

	bool Document::IsValid () const
	{
		return PDocument_ && !PDocument_->isLocked ();
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		DocumentInfo info;
		if (!PDocument_)
			return info;

		info.Title_ = PDocument_->info ("Title");
		info.Subject_ = PDocument_->info ("Subject");
		info.Author_ = PDocument_->info ("Author");
		return info;
	}

	int Document::GetNumPages () const
	{
		return PDocument_->numPages ();
	}

	QSize Document::GetPageSize (int num) const
	{
		std::unique_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return QSize ();

		return page->pageSize ();
	}

	QImage Document::RenderPage (int num, double xScale, double yScale)
	{
		std::unique_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return QImage ();

		return page->renderToImage (72 * xScale, 72 * yScale);
	}

	QList<ILink_ptr> Document::GetPageLinks (int num)
	{
		QList<ILink_ptr> result;
		std::unique_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return result;

		Q_FOREACH (auto link, page->links ())
		{
			if (auto gl = dynamic_cast<Poppler::LinkGoto*> (link))
				result << ILink_ptr (new PageLink (gl));
		}

		return result;
	}
}
}
}
