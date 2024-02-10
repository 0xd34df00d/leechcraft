/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "document.h"
#include <QFile>
#include <QDomDocument>
#include <QtDebug>
#include <util/sll/either.h>
#include <util/monocle/resourcedtextdocument.h>
#include "fb2converter.h"

namespace LC
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

		QDomDocument fb2;
		if (!fb2.setContent (file.readAll (), true))
		{
			qWarning () << Q_FUNC_INFO
					<< "malformed XML in"
					<< filename;
			return;
		}

		auto result = Convert (std::move (fb2));
		Util::Visit (result,
				[this] (const ConvertedDocument& result)
				{
					SetDocument ({
							.BodyElem_ = result.Doc_,
							.TocStructure_ = result.Toc_,
							.Images_ = result.Images_,
							.CoverId_ = result.CoverImageId_,
						});
					Info_ = result.Info_;
				},
				[] (const QString&) {});
	}

	QObject* Document::GetBackendPlugin () const
	{
		return Plugin_;
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		return Info_;
	}

	QUrl Document::GetDocURL () const
	{
		return DocURL_;
	}
}
}
}
