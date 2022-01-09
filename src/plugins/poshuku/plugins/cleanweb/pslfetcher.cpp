/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pslfetcher.h"
#include <chrono>
#include <QDateTime>
#include <QDir>
#include <QNetworkAccessManager>
#include <QTimer>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <util/network/handlenetworkreply.h>
#include "pslhandler.h"

using namespace std::chrono_literals;

namespace LC::Poshuku::CleanWeb
{
	namespace
	{
		QString GetPslPath ()
		{
			const auto& dir = Util::GetUserDir (Util::UserDir::Cache, "poshuku/cleanweb");
			return dir.filePath ("psl.dat");
		}
	}

	PslFetcher::PslFetcher (QNetworkAccessManager& nam, QObject *parent)
	: QObject { parent }
	, PslPath_ { GetPslPath () }
	, NAM_ { nam }
	, Handler_ { std::make_unique<PslHandler> (u""_qsv) }
	{
		QTimer::singleShot (60s, this, &PslFetcher::CheckRefresh);

		if (QFile pslFile { PslPath_ };
			pslFile.exists ())
		{
			if (pslFile.open (QIODevice::ReadOnly))
				Handler_ = std::make_unique<PslHandler> (QString::fromUtf8 (pslFile.readAll ()));
			else
				qWarning () << "unable to open"
						<< PslPath_
						<< "for reading:"
						<< pslFile.errorString ();
		}
	}

	PslFetcher::~PslFetcher () = default;

	const PslHandler& PslFetcher::GetPsl () const
	{
		return *Handler_;
	}

	void PslFetcher::CheckRefresh ()
	{
		QFileInfo fi { PslPath_ };

		const auto& now = QDateTime::currentDateTime ();
		if (fi.exists () && fi.lastModified () >= now.addDays (-1))
			return;

		QNetworkRequest req { QUrl { "https://publicsuffix.org/list/public_suffix_list.dat" } };
		req.setPriority (QNetworkRequest::LowPriority);
		req.setAttribute (QNetworkRequest::RedirectPolicyAttribute, true);

		// no need to pollute the cache with this file, since we're effectively managing the caching ourselves
		req.setAttribute (QNetworkRequest::CacheSaveControlAttribute, false);

		if (fi.exists ())
			req.setHeader (QNetworkRequest::IfModifiedSinceHeader, fi.lastModified ());

		Util::HandleReplySeq<Util::ResultInfo<Util::ReplyWithHeaders>, Util::ErrorInfo<Util::ReplyError>> (NAM_.get (req), this) >>
				Util::Visitor
				{
					[this, now] (const Util::ReplyWithHeaders& reply) { HandleReply (now, reply); },
					[this] (const Util::ReplyError& error)
					{
						qWarning () << "error updating PSL:"
								<< error.HttpStatusCode_
								<< error.Error_
								<< error.ErrorString_;
						QTimer::singleShot (1h, this, &PslFetcher::CheckRefresh);
					}
				};
	}

	void PslFetcher::HandleReply (const QDateTime& now, const Util::ReplyWithHeaders& reply)
	{
		QTimer::singleShot (24h, this, &PslFetcher::CheckRefresh);

		if (reply.Code_ == 304)
		{
			qDebug () << "PSL not modified, skipping";
			return;
		}

		if (reply.Data_.size () < 10000)
		{
			qWarning () << "surprisingly short response:"
					<< reply.Code_
					<< reply.Headers_
					<< reply.Data_;
			return;
		}

		QFile file { PslPath_ };
		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << "unable to open"
					<< PslPath_
					<< "for writing:"
					<< file.errorString ();
			return;
		}

		file.write (reply.Data_);
		file.setFileTime (now, QFileDevice::FileModificationTime);

		Handler_ = std::make_unique<PslHandler> (QString::fromUtf8 (reply.Data_));
	}
}
