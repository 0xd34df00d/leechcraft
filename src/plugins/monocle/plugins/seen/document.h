/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
		void navigateRequested (const QString&, int pageNum, double x, double y);
		void printRequested (const QList<int>&);

		void pageSizeChanged (int);
		void pageContentsChanged (int);
	};
}
}
}
