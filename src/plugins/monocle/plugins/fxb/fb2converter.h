/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <optional>
#include <variant>
#include <QString>
#include <QObject>
#include <QHash>
#include <QStack>
#include <util/sll/eitherfwd.h>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/ihavetoc.h>
#include <util/monocle/textdocumentadapter.h>

class QTextCharFormat;
class QTextBlockFormat;
class QTextCursor;
class QDomElement;
class QDomDocument;
class QTextDocument;

namespace LC
{
namespace Monocle
{
namespace FXB
{
	class Document;

	class CursorCacher;

	class FB2Converter : public QObject
	{
		Document *ParentDoc_;

		const QDomDocument& FB2_;

		std::shared_ptr<QTextDocument> Result_;
		DocumentInfo DocInfo_;
		TOCEntryLevel_t TOC_;

		QStack<TOCEntry*> CurrentTOCStack_;

		std::unique_ptr<QTextCursor> Cursor_;
		std::unique_ptr<CursorCacher> CursorCacher_;

		typedef std::function<void (QDomElement)> Handler_f;
		QHash<QString, Handler_f> Handlers_;

		QSet<QString> UnhandledTags_;
	public:
		struct Config
		{
			QFont DefaultFont_;
			QSize PageSize_;
			QMargins Margins_;

			QColor BackgroundColor_;
			QColor LinkColor_;
		};
	private:
		const Config Config_;
	public:
		struct LinkCtx
		{
			QString Anchor_;
			QPair<int, int> Span_;
		};
	private:
		QList<LinkCtx> LinkSources_;
		QList<LinkCtx> LinkTargets_;
	public:
		FB2Converter (Document*, const QDomDocument&, const Config&);
		~FB2Converter ();

		struct NotAnFBDocument {};
		struct UnsupportedVersion {};

		using Error_t = std::variant<NotAnFBDocument, UnsupportedVersion>;
	private:
		std::optional<Error_t> Error_;
	public:
		struct ConvertedDocument
		{
			std::shared_ptr<QTextDocument> Doc_;
			DocumentInfo Info_;
			TOCEntryLevel_t TOC_;
			QList<TextDocumentAdapter::InternalLink> Links_;
		};
		using ConversionResult_t = Util::Either<Error_t, ConvertedDocument>;
		ConversionResult_t GetResult () const;
	private:
		QList<TextDocumentAdapter::InternalLink> GetLinks () const;

		QDomElement FindBinary (const QString&) const;

		void HandleDescription (const QDomElement&);
		void HandleBody (const QDomElement&);

		void HandleSection (const QDomElement&);
		void HandleTitle (const QDomElement&, int = 0);
		void HandleEpigraph (const QDomElement&);
		void HandleImage (const QDomElement&);

		void HandlePara (const QDomElement&);
		void HandleParaWONL (const QDomElement&);
		void HandlePoem (const QDomElement&);
		void HandleStanza (const QDomElement&);
		void HandleEmptyLine (const QDomElement&);
		void HandleLink (const QDomElement&);

		void HandleChildren (const QDomElement&);
		void Handle (const QDomElement&);

		void HandleMangleBlockFormat (const QDomElement&,
				std::function<void (QTextBlockFormat&)>, Handler_f);
		void HandleMangleCharFormat (const QDomElement&,
				std::function<void (QTextCharFormat&)>, Handler_f);

		void FillPreamble ();
	};
}
}
}
