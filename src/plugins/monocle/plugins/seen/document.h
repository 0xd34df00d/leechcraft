/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QUrl>
#include <QFutureInterface>
#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/idynamicdocument.h>

namespace LC
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
		Q_INTERFACES (LC::Monocle::IDocument LC::Monocle::IDynamicDocument)

		ddjvu_context_t *Context_;
		ddjvu_document_t *Doc_;
		ddjvu_format_t *RenderFormat_;

		DocManager *DocMgr_;

		QVector<QSize> Sizes_;
		QHash<int, ddjvu_page_t*> PendingRenders_;
		QHash<ddjvu_page_t*, int> PendingRendersNums_;

		using RenderJobsPerScale_t = QHash<QPair<double, double>, QFutureInterface<QImage>>;
		using RenderJobs_t = QHash<int, RenderJobsPerScale_t>;
		RenderJobs_t RenderJobs_;

		QSet<int> ScheduledRedraws_;

		QUrl DocURL_;

		QObject *Plugin_;
	public:
		Document (const QString&, ddjvu_context_t*, QObject*, DocManager*);
		~Document ();

		QObject* GetBackendPlugin () const;
		QObject* GetQObject ();
		bool IsValid () const;
		DocumentInfo GetDocumentInfo () const;
		int GetNumPages () const;
		QSize GetPageSize (int) const;
		QFuture<QImage> RenderPage (int, double xRes, double yRes);
		QList<ILink_ptr> GetPageLinks (int);
		QUrl GetDocURL () const;

		ddjvu_document_t* GetNativeDoc () const;

		void UpdateDocInfo ();
		void UpdatePageInfo (ddjvu_page_t*);
		void RedrawPage (ddjvu_page_t*);
	private:
		void ScheduleRedraw (int page, int timeoutHint);
		void TryUpdateSizes ();
		void TryGetPageInfo (int);
		void RunRedrawQueue ();
	signals:
		void printRequested (const QList<int>&);

		void pageSizeChanged (int);
		void pageContentsChanged (int);
	};
}
}
}
