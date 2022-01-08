/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "document.h"
#include <QTimer>
#include <QtConcurrentMap>
#include <util/threads/futures.h>
#include <util/sll/containerconversions.h>
#include <util/sll/qtutil.h>
#include "docmanager.h"

namespace LC
{
namespace Monocle
{
namespace Seen
{
	namespace
	{
		static unsigned int FormatMask [4] = { 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

		bool DebugRedraws ()
		{
			static bool debug = qgetenv ("LC_MONOCLE_SEEN_DEBUG_REDRAWS") == "1";
			return debug;
		}
	}

	Document::Document (const QString& file, ddjvu_context_t *ctx, QObject *plugin, DocManager *mgr)
	: Context_ (ctx)
	, Doc_ (ddjvu_document_create_by_filename_utf8 (Context_, file.toUtf8 ().constData (), 1))
	, RenderFormat_ (ddjvu_format_create (DDJVU_FORMAT_RGBMASK32, 4, FormatMask))
	, DocMgr_ (mgr)
	, DocURL_ (QUrl::fromLocalFile (file))
	, Plugin_ (plugin)
	{
		ddjvu_format_set_row_order (RenderFormat_, 1);
		ddjvu_format_set_y_direction (RenderFormat_, 1);

		if (Doc_ && ddjvu_document_get_type (Doc_) != DDJVU_DOCTYPE_UNKNOWN)
			UpdateDocInfo ();
	}

	Document::~Document ()
	{
		ddjvu_format_release (RenderFormat_);
		DocMgr_->Unregister (Doc_);
		ddjvu_document_release (Doc_);
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
		return Doc_;
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		return {};
	}

	int Document::GetNumPages () const
	{
		return ddjvu_document_get_pagenum (Doc_);
	}

	QSize Document::GetPageSize (int pageNum) const
	{
		return Sizes_.value (pageNum);
	}

	QFuture<QImage> Document::RenderPage (int pageNum, double xScale, double yScale)
	{
		if (DebugRedraws ())
			qDebug () << Q_FUNC_INFO << pageNum << xScale << yScale;
		const auto& size = Sizes_.value (pageNum);

		if (std::max (xScale, yScale) < 0.01)
		{
			QImage img { size, QImage::Format_RGB32 };
			const auto& scaled = img.scaled (img.width () * xScale, img.height () * yScale,
					Qt::KeepAspectRatio, Qt::SmoothTransformation);
			return Util::MakeReadyFuture (scaled);
		}

		if (!PendingRenders_.contains (pageNum))
		{
			const auto page = ddjvu_page_create_by_pageno (Doc_, pageNum);
			PendingRenders_ [pageNum] = page;
			PendingRendersNums_ [page] = pageNum;

			ScheduleRedraw (pageNum, 100);
		}

		return RenderJobs_ [pageNum] [{ xScale, yScale }].future ();
	}

	QList<ILink_ptr> Document::GetPageLinks (int)
	{
		return {};
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
		if (!PendingRendersNums_.contains (page))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown page";
			return;
		}

		ScheduleRedraw (PendingRendersNums_ [page], 100);
	}

	void Document::ScheduleRedraw (int page, int timeoutHint)
	{
		if (ScheduledRedraws_.isEmpty ())
			QTimer::singleShot (timeoutHint, this, &Document::RunRedrawQueue);
		ScheduledRedraws_ << page;
	}

	void Document::TryUpdateSizes ()
	{
		const int numPages = GetNumPages ();
		Sizes_.resize (numPages);
		for (int i = 0; i < numPages; ++i)
			if (!Sizes_.at (i).isValid ())
				TryGetPageInfo (i);
	}

	void Document::TryGetPageInfo (int pageNum)
	{
		if (pageNum >= Sizes_.size ())
		{
			qWarning () << Q_FUNC_INFO
					<< "page out of bounds:"
					<< pageNum
					<< Sizes_;
			return;
		}

		ddjvu_pageinfo_t info;
		auto r = ddjvu_document_get_pageinfo (Doc_, pageNum, &info);
		if (r != DDJVU_JOB_OK)
			return;

		Sizes_ [pageNum] = QSize (info.width, info.height);
		emit pageSizeChanged (pageNum);
	}

	void Document::RunRedrawQueue ()
	{
		if (DebugRedraws ())
			qDebug () << Q_FUNC_INFO << ScheduledRedraws_;
		if (ScheduledRedraws_.empty ())
			return;

		struct PageRedrawContext
		{
			int PageNum_;
			ddjvu_page_t *Page_;
			RenderJobsPerScale_t RenderJobs_;
			QSize SrcSize_;
		};

		QList<PageRedrawContext> pages;
		for (const auto num : ScheduledRedraws_)
			pages.push_back ({ num, PendingRenders_ [num], RenderJobs_.take (num), Sizes_.value (num) });

		ScheduledRedraws_.clear ();

		struct Result
		{
			RenderJobs_t Unrendered_;
		};

		const auto& future = QtConcurrent::mappedReduced (pages,
				std::function<Result (PageRedrawContext)>
				{
					[fmt = RenderFormat_] (const PageRedrawContext& ctx)
					{
						const auto& srcSize = ctx.SrcSize_;

						Result result;
						for (const auto& pair : Util::Stlize (ctx.RenderJobs_))
						{
							const auto& scale = pair.first;
							const auto& size = srcSize.scaled (srcSize.width () * scale.first,
									srcSize.height () * scale.second,
									Qt::KeepAspectRatio);

							QImage img { size, QImage::Format_RGB32 };

							ddjvu_rect_s rect
							{
								0,
								0,
								static_cast<unsigned int> (size.width ()),
								static_cast<unsigned int> (size.height ())
							};

							int res = 0;
							int retries = 0;
							do
							{
								res = ddjvu_page_render (ctx.Page_,
										DDJVU_RENDER_COLOR,
										&rect,
										&rect,
										fmt,
										img.bytesPerLine (),
										reinterpret_cast<char*> (img.bits ()));
							} while (res == DDJVU_JOB_STARTED && ++retries < 3);

							if (DebugRedraws ())
								qDebug () << Q_FUNC_INFO << ctx.PageNum_ << res;

							if (res == DDJVU_JOB_OK || res == DDJVU_JOB_STARTED || res == DDJVU_JOB_NOTSTARTED)
							{
								auto future = pair.second;
								if (res == DDJVU_JOB_NOTSTARTED)
									img.fill (Qt::white);
								Util::ReportFutureResult (future, img);
							}
							else
								result.Unrendered_ [ctx.PageNum_] [scale] = pair.second;
						}
						return result;
					}
				},
				+[] (Result& acc, const Result& partial) { acc.Unrendered_.insert (partial.Unrendered_); });

		Util::Sequence (this, future) >>
				[this] (const Result& result)
				{
					RenderJobs_.insert (result.Unrendered_);

					const auto& remainingJobs = Util::AsSet (RenderJobs_.keys ());

					const auto finishedPages = [this, &remainingJobs]
					{
						auto finishedPages = Util::AsSet (PendingRenders_.keys ());
						return finishedPages.subtract (remainingJobs);
					} ();

					if (DebugRedraws ())
					{
						qDebug () << Q_FUNC_INFO << "cleaning up finished" << finishedPages;
						qDebug () << Q_FUNC_INFO << "remaining pages:" << remainingJobs;
					}

					for (const auto num : finishedPages)
					{
						const auto page = PendingRenders_.take (num);
						PendingRendersNums_.remove (page);
						ddjvu_page_release (page);
					}

					if (!remainingJobs.isEmpty ())
					{
						ScheduledRedraws_ += remainingJobs;
						QTimer::singleShot (100, this, &Document::RunRedrawQueue);
					}
				};
	}
}
}
}
