/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textdocumentadapter.h"
#include <cmath>
#include <QDomElement>
#include <QGuiApplication>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextEdit>
#include <util/sll/timer.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include "html2doc.h"
#include "textdocumentformatconfig.h"

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
		void AddCoverImage (QTextDocument& doc, const TextDocumentAdapter::ImagesList_t& images, const QString& coverId)
		{
			if (coverId.isEmpty ())
				return;

			const auto imagePos = std::find_if (images.begin (), images.end (),
					[&] (const auto& pair) { return pair.first == coverId; });
			if (imagePos == images.end ())
			{
				qWarning () << "unknown cover image"
						<< coverId;
				return;
			}

			const auto& upscaledId = coverId + ".upscaled";

			QTextCursor cursor { &doc };
			cursor.insertHtml ("<img src='%1' />"_qs.arg (upscaledId));

			const auto& pageSize = doc.pageSize ().toSize ();
			const auto& scaled = imagePos->second.scaled (pageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			doc.addResource (QTextDocument::ImageResource, { upscaledId }, scaled);
		}
	}

	void TextDocumentAdapter::SetDocument (const QDomElement& elem, const ImagesList_t& images, const QString& coverId)
	{
		Doc_ = std::make_unique<QTextDocument> ();
		TextDocumentFormatConfig::Instance ().FormatDocument (*Doc_);
		Util::Timer timer;
		Html2Doc (*Doc_, elem);
		timer.Stamp ("html2doc");

		AddCoverImage (*Doc_, images, coverId);

		for (const auto& [id, image] : images)
			Doc_->addResource (QTextDocument::ImageResource, { id }, QVariant::fromValue (image));
	}

}
