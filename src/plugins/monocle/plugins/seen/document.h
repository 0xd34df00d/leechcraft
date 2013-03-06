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

#pragma once

#include <QObject>
#include <QHash>
#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/idynamicdocument.h>

namespace LeechCraft
{
namespace Monocle
{
namespace Seen
{
	class DocManager;

	class Document : public QObject
				   , public IDocument
				   , public IDynamicDocument
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Monocle::IDocument LeechCraft::Monocle::IDynamicDocument)

		ddjvu_context_t *Context_;
		ddjvu_document_t *Doc_;
		ddjvu_format_t *RenderFormat_;

		DocManager *DocMgr_;

		QHash<int, QSize> Sizes_;
		QHash<int, ddjvu_page_t*> PendingRenders_;
		QHash<ddjvu_page_t*, int> PendingRendersNums_;
	public:
		Document (const QString&, ddjvu_context_t*, DocManager*);
		~Document ();

		QObject* GetObject ();
		bool IsValid () const;
		DocumentInfo GetDocumentInfo () const;
		int GetNumPages () const;
		QSize GetPageSize (int) const;
		QImage RenderPage (int, double xRes, double yRes);
		QList<ILink_ptr> GetPageLinks (int);

		ddjvu_document_t* GetNativeDoc () const;

		void UpdateDocInfo ();
		void UpdatePageInfo (ddjvu_page_t*);
		void RedrawPage (ddjvu_page_t*);
	private:
		void TryUpdateSizes ();
		void TryGetPageInfo (int);
	signals:
		void navigateRequested (const QString&, int pageNum, double x, double y);

		void pageSizeChanged (int);
		void pageContentsChanged (int);
	};
}
}
}
