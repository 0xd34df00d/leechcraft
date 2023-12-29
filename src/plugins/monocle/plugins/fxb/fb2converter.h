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
#include <util/monocle/textdocumentformatconfig.h>

class QTextCharFormat;
class QTextBlockFormat;
class QTextCursor;
class QDomElement;
class QDomDocument;
class QTextDocument;

namespace LC::Monocle::FXB
{
	class Document;

	class CursorCacher;

	class FB2Converter : public QObject
	{
		Document *ParentDoc_;

		const QDomDocument& FB2_;

		std::unique_ptr<QTextDocument> Result_;
		DocumentInfo DocInfo_;
		TOCEntryLevel_t TOC_;

		QStack<TOCEntry*> CurrentTOCStack_;

		std::unique_ptr<QTextCursor> Cursor_;
		std::unique_ptr<CursorCacher> CursorCacher_;

		using Handler_f = std::function<void (QDomElement)>;
		QHash<QStringView, Handler_f> Handlers_;

		QHash<QString, int> UnhandledTags_;

		const TextDocumentPalette Palette_;
	public:
		struct LinkCtx
		{
			QString Anchor_;
			QPair<int, int> Span_;
		};
	private:
		QVector<LinkCtx> LinkSources_;
		QVector<LinkCtx> LinkTargets_;
	public:
		FB2Converter (Document*, const QDomDocument&);
		~FB2Converter () override;

		struct NotAnFBDocument {};
		struct UnsupportedVersion {};

		using Error_t = std::variant<NotAnFBDocument, UnsupportedVersion>;
	private:
		std::optional<Error_t> Error_;
	public:
		struct ConvertedDocument
		{
			std::unique_ptr<QTextDocument> Doc_;
			DocumentInfo Info_;
			TOCEntryLevel_t TOC_;
			QVector<TextDocumentAdapter::InternalLink> Links_;
		};
		using ConversionResult_t = Util::Either<Error_t, ConvertedDocument>;
		ConversionResult_t GetResult () &&;
	private:
		QVector<TextDocumentAdapter::InternalLink> GetLinks () const;

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
				const std::function<void (QTextBlockFormat&)>&, const Handler_f&);
		void HandleMangleCharFormat (const QDomElement&,
				const std::function<void (QTextCharFormat&)>&, const Handler_f&);

		void FillPreamble ();
	};
}
