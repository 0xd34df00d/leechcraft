/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mailwebpagenam.h"
#include <QFile>
#include <util/network/customnetworkreply.h>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include "accountthread.h"
#include "attachmentsfetcher.h"
#include "mailwebpage.h"

namespace LC::Snails
{
	MailWebPageNAM::MailWebPageNAM (ContextGetter getter, QObject *parent)
	: QNetworkAccessManager { parent }
	, CtxGetter_ { std::move (getter) }
	{
	}

	QNetworkReply* MailWebPageNAM::createRequest (Operation op, const QNetworkRequest& origReq, QIODevice *dev)
	{
		const auto& scheme = origReq.url ().scheme ();
		if (scheme == "http" || scheme == "https")
			return HandleNetworkRequest (op, origReq, dev);
		if (scheme == "cid")
			return HandleCIDRequest (op, origReq, dev);

		return QNetworkAccessManager::createRequest (op, origReq, dev);
	}

	QNetworkReply* MailWebPageNAM::HandleNetworkRequest (Operation, const QNetworkRequest& origReq, QIODevice*)
	{
		const auto reply = new Util::CustomNetworkReply { origReq.url (), this };
		reply->SetContent (QByteArray { "Blocked" });
		reply->setError (QNetworkReply::ContentAccessDenied,
				QString { "Blocked: %1" }.arg (origReq.url ().toString ()));
		return reply;
	}

	QNetworkReply* MailWebPageNAM::HandleCIDRequest (Operation op, const QNetworkRequest& origReq, QIODevice*)
	{
		const auto reply = new Util::CustomNetworkReply { origReq.url (), this };

		if (op != GetOperation)
		{
			qWarning () << Q_FUNC_INFO
					<< "unsupported operation"
					<< op
					<< origReq.url ();

			reply->SetContent (QByteArray { "Unsupported operation" });
			reply->setError (QNetworkReply::ContentOperationNotPermittedError,
					QString { "Unsupported operation %1 on %2" }
						.arg (op)
						.arg (origReq.url ().toString ()));
			return reply;
		}

		const auto& reqPath = origReq.url ().path ();

		const auto& ctx = CtxGetter_ ();
		// FIXME
		// This shall be cid-based, but we don't necessarily have cid at this point,
		// so let's use the next best approximation.
		const auto& atts = ctx.MsgInfo_.Attachments_;
		const auto attPos = std::find_if (atts.begin (), atts.end (),
				[&reqPath] (const AttDescr& att)
				{
					const auto& name = att.GetName ();
					return !name.isEmpty () &&
							(name.contains (reqPath) || reqPath.contains (name));
				});

		if (attPos == atts.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "attachment"
					<< reqPath
					<< "not found among"
					<< Util::Map (atts, &AttDescr::GetName);

			reply->SetContent (QByteArray { "Attachment not found" });
			reply->setError (QNetworkReply::ContentNotFoundError,
					QString { "Attachment %1 not found" }
							.arg (origReq.url ().toString ()));

			return reply;
		}

		qDebug () << Q_FUNC_INFO
				<< "fetching attachment"
				<< attPos->GetName ();

		const auto af = std::make_shared<AttachmentsFetcher> (ctx.Acc_,
				ctx.MsgInfo_.Folder_, ctx.MsgInfo_.FolderId_, QStringList { attPos->GetName () });

		reply->setHeader (QNetworkRequest::ContentLengthHeader, attPos->GetSize ());
		reply->setHeader (QNetworkRequest::ContentTypeHeader, attPos->GetType () + '/' + attPos->GetSubType ());

		Util::Sequence (reply, af->GetFuture ()) >>
				Util::Visitor
				{
					[reply, af] (const AttachmentsFetcher::FetchResult& result)
					{
						qDebug () << Q_FUNC_INFO << "done fetching attachment into" << result.Paths_;

						QFile file { result.Paths_.value (0) };
						file.open (QIODevice::ReadOnly);
						reply->SetContent (file.readAll ());
					},
					[reply, af] (auto)
					{
						reply->SetContent (QByteArray { "Unable to fetch attachment" });
						reply->setError (QNetworkReply::InternalServerError,
								QString { "Unable to fetch attachment" });
					}
				};

		return reply;
	}
}
