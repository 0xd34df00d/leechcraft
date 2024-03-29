/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDomElement>
#include <QString>
#include <util/sll/eitherfwd.h>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/ihavetoc.h>
#include <util/monocle/types.h>

class QTextDocument;

namespace LC::Monocle::FXB
{
	struct ConvertedDocument
	{
		QDomElement Doc_;
		TOCEntryID Toc_;
		LazyImages_t Images_;
		QString CoverImageId_;

		DocumentInfo Info_;
	};

	using ConvertResult_t = Util::Either<QString, ConvertedDocument>;
	ConvertResult_t Convert (QDomDocument&& fb2);
}
