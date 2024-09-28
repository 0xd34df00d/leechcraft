/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textdocumentadapter.h"
#include <cmath>
#include <QAbstractTextDocumentLayout>
#include <QDomElement>
#include <QGuiApplication>
#include <QPainter>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextEdit>
#include <util/sll/timer.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include "html2doc.h"
#include "pagelink.h"
#include "resourcedtextdocument.h"
#include "textdocumentformatconfig.h"

namespace LC::Monocle
{
	TextDocumentAdapter::~TextDocumentAdapter () = default;

	QObject* TextDocumentAdapter::GetQObject ()
	{
		return this;
	}

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

	const DocumentSignals* TextDocumentAdapter::GetDocumentSignals () const
	{
		return nullptr;
	}

	namespace
	{
		QPointF ToDocPoint (int page, QPointF rel, QSizeF pageSize)
		{
			return { rel.x () * pageSize.width (), (page + rel.y ()) * pageSize.height () };
		}

		int GetCursorPosition (int page, QPointF rel, QTextDocument& doc)
		{
			const auto docPoint = ToDocPoint (page, rel, doc.pageSize ());
			return doc.documentLayout ()->hitTest (docPoint, Qt::FuzzyHit);
		}
	}

	QString TextDocumentAdapter::GetTextContent (int page, const PageRelativeRectBase& rect)
	{
		const auto startPos = GetCursorPosition (page, rect.R_.topLeft (), *Doc_);
		const auto endPos = GetCursorPosition (page, rect.R_.bottomRight (), *Doc_);
		if (startPos < 0 || endPos < 0 || startPos >= endPos)
		{
			qWarning () << "invalid positions" << page << rect.R_ << startPos << endPos;
			return {};
		}

		QTextCursor cursor { Doc_.get () };
		cursor.setPosition (startPos, QTextCursor::MoveAnchor);
		cursor.setPosition (endPos, QTextCursor::KeepAnchor);
		return cursor.selectedText ();
	}

	QVector<TextBox> TextDocumentAdapter::GetTextBoxes (int page)
	{
		return {};
	}

	TOCEntryLevel_t TextDocumentAdapter::GetTOC ()
	{
		return TOC_;
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

			QVector<QPair<int, PageRelativeRectBase>> result;
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
				result << QPair { pageNum, PageRelativeRectBase { scale.mapRect (bounding) } };
			}
			return result;
		}

		template<typename T>
		QMap<int, QList<T>> ListToMap (const QVector<QPair<int, T>>& list)
		{
			QMap<int, QList<T>> result;
			for (const auto& [page, rect] : list)
				result [page] << rect;
			return result;
		}
	}

	QMap<int, QList<PageRelativeRectBase>> TextDocumentAdapter::GetTextPositions (const QString& text, Qt::CaseSensitivity cs)
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
		void AddCoverImage (QTextDocument& doc, const LazyImages_t& images, const QString& coverId)
		{
			if (coverId.isEmpty ())
				return;

			const auto& image = images.value (coverId);
			if (!image)
			{
				qWarning () << "unknown cover image"
						<< coverId;
				return;
			}

			const auto& upscaledId = coverId + ".upscaled";

			QTextCursor cursor { &doc };
			cursor.insertHtml ("<img src='%1' />"_qs.arg (upscaledId));

			const auto& pageSize = doc.pageSize ().toSize ();
			doc.addResource (QTextDocument::ImageResource, { upscaledId }, image.Load_ (pageSize));
		}

		QHash<int, QList<ILink_ptr>> CreateLinks (const QTextDocument& textDoc, const QVector<InternalLink>& intLinks)
		{
			QHash<int, QList<ILink_ptr>> result;
			for (const auto& intLink : intLinks)
			{
				auto pageLink = std::make_shared<PageLink> (PageLink::LinkInfo
						{
							.TextDoc_ = textDoc,
							.Target_ = intLink.Target_,
							.Source_ = intLink.Link_,
							.ToolTip_ = intLink.LinkTitle_,
						});
				result [pageLink->GetSourcePage ()] << pageLink;
			}
			return result;
		}

		TOCEntryLevel_t MaterializeSpans (const TOCEntryLevelT<Span>& toc, QTextDocument& doc)
		{
			TOCEntryLevel_t result;
			result.reserve (toc.size ());
			for (const auto& spanned : toc)
			{
				const auto& nav = PageLink::GetNavigationAction ({
						.TextDoc_ = doc,
						.Target_ = spanned.Navigation_,
					});
				result.append ({
						.Navigation_ = nav,
						.Name_ = spanned.Name_,
						.ChildLevel_ = MaterializeSpans (spanned.ChildLevel_, doc),
					});
			}
			return result;
		}
	}

	void TextDocumentAdapter::SetDocument (const HtmlDocument& info)
	{
		Doc_ = std::make_unique<ResourcedTextDocument> (info.Images_);
		TextDocumentFormatConfig::Instance ().FormatDocument (*Doc_);
		Util::Timer timer;
		auto docStructure = Html2Doc ({ *Doc_, info.BodyElem_, info.TocStructure_, info.Styler_, info.Images_ });
		timer.Stamp ("html2doc");

		AddCoverImage (*Doc_, info.Images_, info.CoverId_);
		timer.Stamp ("covers and resources");
		Doc_->documentLayout ();
		timer.Stamp ("layout");

		TOC_ = MaterializeSpans (docStructure.TOC_, *Doc_);
		timer.Stamp ("toc");

		Links_ = CreateLinks (*Doc_, docStructure.InternalLinks_);
		timer.Stamp ("links creation");
	}
}
