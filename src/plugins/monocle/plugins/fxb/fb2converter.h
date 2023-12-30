/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <util/sll/eitherfwd.h>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/ihavetoc.h>
#include <util/monocle/textdocumentadapter.h>

class QDomDocument;
class QTextDocument;

namespace LC::Monocle::FXB
{
	struct ConvertedDocument
	{
		std::unique_ptr<QTextDocument> Doc_;
		DocumentInfo Info_;
		TOCEntryLevel_t TOC_;
		QVector<TextDocumentAdapter::InternalLink> Links_;
	};

	using ConvertResult_t = Util::Either<QString, ConvertedDocument>;
	ConvertResult_t Convert (QDomDocument&& fb2);
}
