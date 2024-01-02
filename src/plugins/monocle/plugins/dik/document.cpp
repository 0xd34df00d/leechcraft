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
#include <QTextDocument>
#include <QTextFrameFormat>
#include <QTextFrame>
#include "mobiparser.h"

namespace LC
{
namespace Monocle
{
namespace Dik
{
	namespace
	{
		class MobiTextDocument : public QTextDocument
		{
			const std::shared_ptr<MobiParser> P_;
		public:
			MobiTextDocument (const std::shared_ptr<MobiParser>& p, QObject *parent = 0)
			: QTextDocument (parent)
			, P_ (p)
			{
			}
		protected:
			QVariant loadResource (int type, const QUrl& name);
		};

		QVariant MobiTextDocument::loadResource (int type, const QUrl& name)
		{
			if (type != ImageResource || name.scheme () != "rec")
				return QTextDocument::loadResource (type, name);

			bool ok = false;
			const quint16 recnum = static_cast<quint16> (name.path ().mid (1).toUInt (&ok));
			if (!ok)
				return {};

			QVariant resource;
			resource.setValue (P_->GetImage (recnum - 1));
			addResource (type, name, resource);

			return resource;
		}

		QString Fix (QString markup)
		{
			QRegExp imgRx ("<img.*recindex=\"([\\d]*)\".*>", Qt::CaseInsensitive);
			imgRx.setMinimal (true);
			markup.replace (imgRx, "<img src='rec:/\\1' />");

			markup.replace ("<mbp:pagebreak/>", "<p style='page-break-after: always' />");

			return markup;
		}
	}

	Document::Document (const QString& filename, QObject *plugin)
	: DocURL_ (QUrl::fromLocalFile (filename))
	, Parser_ (new MobiParser (filename))
	, Plugin_ (plugin)
	{
		if (!Parser_->IsValid ())
			return;

		QString contents;
		try
		{
			contents = Parser_->GetText ();
		}
		catch (const std::exception&)
		{
			return;
		}

		auto doc = std::make_unique<MobiTextDocument> (Parser_);
		doc->setPageSize (QSize (600, 800));
		doc->setUndoRedoEnabled (false);

		if (contents.contains ("<html", Qt::CaseInsensitive))
			doc->setHtml (Fix (contents));
		else
			doc->setPlainText (contents);

		QTextFrameFormat format;
		format.setMargin (30);
		doc->rootFrame ()->setFrameFormat (format);

		// TODO
		// SetDocument (std::move (doc), {});
		Info_ = Parser_->GetDocInfo ();
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
