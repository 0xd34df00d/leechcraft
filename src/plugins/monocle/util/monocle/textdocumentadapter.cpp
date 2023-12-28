/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textdocumentadapter.h"
#include <cmath>
#include <QGuiApplication>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextEdit>
#include <util/threads/futures.h>

namespace LC::Monocle
{
	TextDocumentAdapter::~TextDocumentAdapter () = default;

	bool TextDocumentAdapter::IsValid () const
	{
		return static_cast<bool> (Doc_);
	}

	int TextDocumentAdapter::GetNumPages () const
	{
		return Doc_->pageCount ();
	}

	QSize TextDocumentAdapter::GetPageSize (int) const
	{
		auto size = Doc_->pageSize ();
		size.setWidth (std::ceil (size.width ()));
		return size.toSize ();
	}

	namespace
	{
		constexpr auto DefaultHints = QPainter::RenderHint::Antialiasing |
				QPainter::RenderHint::TextAntialiasing |
				QPainter::RenderHint::SmoothPixmapTransform;
	}

	QFuture<QImage> TextDocumentAdapter::RenderPage (int page, double xScale, double yScale)
	{
		const auto& size = Doc_->pageSize ();

		auto imgSize = size.toSize ();
		imgSize.rwidth () *= xScale;
		imgSize.rheight () *= yScale;
		QImage image (imgSize, QImage::Format_ARGB32);
		image.fill (Qt::white);

		QRectF rect (QPointF (0, 0), size);
		rect.moveTop (rect.height () * page);

		QPainter painter;
		painter.begin (&image);
		painter.setRenderHints (DefaultHints);
		painter.scale (xScale, yScale);
		painter.translate (0, rect.height () * (-page));
		Doc_->drawContents (&painter, rect);
		painter.end ();

		return Util::MakeReadyFuture (image);
	}

	QList<ILink_ptr> TextDocumentAdapter::GetPageLinks (int page)
	{
		return Links_.value (page);
	}

	void TextDocumentAdapter::PaintPage (QPainter *painter, int page, double xScale, double yScale)
	{
		painter->save ();
		const auto oldHints = painter->renderHints ();
		painter->setRenderHints (DefaultHints);

		painter->scale (xScale, yScale);

		const auto& size = Doc_->pageSize ();

		QRectF rect (QPointF (0, 0), size);
		rect.moveTop (rect.height () * page);
		Doc_->drawContents (painter, rect);

		painter->setRenderHints (oldHints);
		painter->restore ();
	}

	namespace
	{
		auto GetCursorsPositions (QTextDocument& doc, const QVector<QPair<QTextCursor, QTextCursor>>& cursors)
		{
			const auto& pageSize = doc.pageSize ();
			const auto pageHeight = pageSize.height ();

			QTextEdit hackyEdit;
			hackyEdit.setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
			hackyEdit.setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
			hackyEdit.setFixedSize (doc.pageSize ().toSize ());
			hackyEdit.setDocument (&doc);
			doc.setPageSize (pageSize);

			const QMatrix scale = QMatrix {}.scale (1 / pageSize.width (), 1 / pageSize.height ());

			QVector<QPair<int, QRectF>> result;
			result.reserve (cursors.size ());
			for (const auto& pair : cursors)
			{
				auto rect = hackyEdit.cursorRect (pair.first);
				auto endRect = hackyEdit.cursorRect (pair.second);

				const int pageNum = rect.y () / pageHeight;
				rect.moveTop (rect.y () - pageHeight * pageNum);
				endRect.moveTop (endRect.y () - pageHeight * pageNum);

				if (rect.y () != endRect.y ())
				{
					rect.setWidth (pageSize.width () - rect.x ());
					endRect.setX (0);
				}
				QRectF bounding { rect | endRect };

				bounding = scale.mapRect (bounding);

				result << QPair { pageNum, bounding };
			}
			return result;
		}

		QMap<int, QList<QRectF>> ListToMap (const QVector<QPair<int, QRectF>>& list)
		{
			QMap<int, QList<QRectF>> result;
			for (const auto& [page, rect] : list)
				result [page] << rect;
			return result;
		}
	}

	QMap<int, QList<QRectF>> TextDocumentAdapter::GetTextPositions (const QString& text, Qt::CaseSensitivity cs)
	{
		QVector<QPair<QTextCursor, QTextCursor>> cursors;

		const auto tdFlags = cs == Qt::CaseSensitive ?
				QTextDocument::FindCaseSensitively :
				QTextDocument::FindFlags ();
		auto cursor = Doc_->find (text, 0, tdFlags);
		while (!cursor.isNull ())
		{
			auto startCursor = cursor;
			startCursor.setPosition (cursor.selectionStart ());
			cursors << QPair { startCursor, cursor };
			cursor = Doc_->find (text, cursor, tdFlags);
		}

		return ListToMap (GetCursorsPositions (*Doc_, cursors));
	}

	namespace
	{
		class Link : public ILink
				   , public IPageLink
		{
			const QRectF LinkArea_;

			const int TargetPage_;
			const QRectF TargetArea_;

			IDocument * const Doc_;
		public:
			Link (const QRectF& area, int targetPage, const QRectF& targetArea, IDocument *doc)
			: LinkArea_ { area }
			, TargetPage_ { targetPage }
			, TargetArea_ { targetArea }
			, Doc_ { doc }
			{
			}

			LinkType GetLinkType () const override
			{
				return LinkType::PageLink;
			}

			QRectF GetArea () const override
			{
				return LinkArea_;
			}

			void Execute () override
			{
				Doc_->navigateRequested ({}, { TargetPage_, TargetArea_.topLeft () });
			}

			QString GetDocumentFilename () const override
			{
				return {};
			}

			int GetPageNumber () const override
			{
				return TargetPage_;
			}

			double NewX () const override
			{
				return TargetArea_.left ();
			}

			double NewY () const override
			{
				return TargetArea_.top ();
			}

			double NewZoom () const override
			{
				return 0;
			}
		};
	}

	void TextDocumentAdapter::SetDocument (std::unique_ptr<QTextDocument> doc, const QVector<InternalLink>& links)
	{
		Doc_ = std::move (doc);

		Links_.clear ();
		if (links.isEmpty ())
			return;

		const auto makeCursor = [this] (int position)
		{
			QTextCursor cur { &*Doc_ };
			cur.setPosition (position);
			return cur;
		};

		QVector<QPair<QTextCursor, QTextCursor>> srcCursors;
		srcCursors.reserve (links.size ());
		QVector<QPair<QTextCursor, QTextCursor>> dstCursors;
		dstCursors.reserve (links.size ());
		for (const auto& link : links)
		{
			srcCursors.push_back ({ makeCursor (link.FromSpan_.first), makeCursor (link.FromSpan_.second) });
			dstCursors.push_back ({ makeCursor (link.ToSpan_.first), makeCursor (link.ToSpan_.second) });
		}

		const auto& srcPositions = GetCursorsPositions (*Doc_, srcCursors);
		const auto& dstPositions = GetCursorsPositions (*Doc_, dstCursors);

		for (int i = 0; i < srcPositions.size () && i < dstPositions.size (); ++i)
		{
			const auto& srcPos = srcPositions.at (i);
			const auto& dstPos = dstPositions.at (i);
			Links_ [srcPos.first] << std::make_shared<Link> (srcPos.second, dstPos.first, dstPos.second, this);
		}
	}
}
