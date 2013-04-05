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
#include <QFile>
#include <QDomDocument>
#include <QtDebug>
#include <QTextDocument>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QTextEdit>
#include "fb2converter.h"

namespace LeechCraft
{
namespace Monocle
{
namespace FXB
{
	Document::Document (const QString& filename, QObject *plugin)
	: DocURL_ (QUrl::fromLocalFile (filename))
	, Plugin_ (plugin)
	{
		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		QDomDocument doc;
		if (!doc.setContent (file.readAll (), true))
		{
			qWarning () << Q_FUNC_INFO
					<< "malformed XML in"
					<< filename;
			return;
		}

		FB2Converter conv (this, doc);
		auto textDoc = conv.GetResult ();
		SetDocument (textDoc);
		Info_ = conv.GetDocumentInfo ();
		TOC_ = conv.GetTOC ();
	}

	QObject* Document::GetBackendPlugin () const
	{
		return Plugin_;
	}

	QObject* Document::GetQObject ()
	{
		return this;
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		return Info_;
	}

	QUrl Document::GetDocURL () const
	{
		return DocURL_;
	}

	TOCEntryLevel_t Document::GetTOC ()
	{
		return TOC_;
	}

	QMap<int, QList<QRectF>> Document::GetTextPositions (const QString& text, Qt::CaseSensitivity cs)
	{
		const auto& pageSize = Doc_->pageSize ();
		const auto pageHeight = pageSize.height ();

		QTextEdit hackyEdit;
		hackyEdit.setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		hackyEdit.setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		hackyEdit.setFixedSize (Doc_->pageSize ().toSize ());
		hackyEdit.setDocument (Doc_.get ());
		Doc_->setPageSize (pageSize);

		const auto tdFlags = cs == Qt::CaseSensitive ?
				QTextDocument::FindCaseSensitively :
				QTextDocument::FindFlags ();

		QMap<int, QList<QRectF>> result;
		auto cursor = Doc_->find (text, 0, tdFlags);
		while (!cursor.isNull ())
		{
			auto endRect = hackyEdit.cursorRect (cursor);
			auto startCursor = cursor;
			startCursor.setPosition (cursor.selectionStart ());
			auto rect = hackyEdit.cursorRect (startCursor);

			const int pageNum = rect.y () / pageHeight;
			rect.moveTop (rect.y () - pageHeight * pageNum);
			endRect.moveTop (endRect.y () - pageHeight * pageNum);

			if (rect.y () != endRect.y ())
			{
				rect.setWidth (pageSize.width () - rect.x ());
				endRect.setX (0);
			}
			auto bounding = rect | endRect;

			result [pageNum] << bounding;

			cursor = Doc_->find (text, cursor, tdFlags);
		}
		return result;
	}

	void Document::RequestNavigation (int page)
	{
		emit navigateRequested (QString (), page, 0, 0.4);
	}
}
}
}
