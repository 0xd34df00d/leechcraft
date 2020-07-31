/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audiosearch.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <util/sll/queuemanager.h>
#include <util/sll/urloperator.h>
#include <util/sll/visitor.h>
#include <util/sll/parsejson.h>
#include <util/threads/futures.h>
#include <util/network/handlenetworkreply.h>
#include <util/svcauth/vkauthmanager.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace TouchStreams
{
	AudioSearch::AudioSearch (ICoreProxy_ptr proxy, const Media::AudioSearchRequest& query,
			Util::SvcAuth::VkAuthManager *mgr, Util::QueueManager *queue, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Queue_ (queue)
	, Query_ (query)
	{
		Promise_.reportStarted ();

		Util::Sequence (this, mgr->GetAuthKeyFuture ()) >>
				Util::Visitor
				{
					[this] (const QString& key) { HandleGotAuthKey (key); },
					Util::Visitor
					{
						[this] (Util::SvcAuth::VkAuthManager::SilentMode)
						{
							Util::ReportFutureResult (Promise_, "VK authenticator is in silent mode.");
						}
					}
				};
	}

	QFuture<Media::IAudioPile::Result_t> AudioSearch::GetFuture ()
	{
		return Promise_.future ();
	}

	void AudioSearch::HandleGotAuthKey (const QString& key)
	{
		QUrl url ("https://api.vk.com/method/audio.search");
		Util::UrlOperator { url }
				("access_token", key)
				("q", Query_.FreeForm_)
				("count", XmlSettingsManager::Instance ().property ("SearchResultsCount").toInt ());

		Queue_->Schedule ([this, url]
				{
					auto reply = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (url));
					Util::HandleReplySeq (reply, this) >>
							Util::Visitor
							{
								[this] (Util::Void) { Util::ReportFutureResult (Promise_, "Unable to request audio search."); },
								[this] (const QByteArray& data) { HandleGotReply (data); }
							}.Finally ([this] { deleteLater (); });
				},
				this,
				Util::QueuePriority::High);
	}

	void AudioSearch::HandleGotReply (const QByteArray& data)
	{
		Media::IAudioPile::Results_t results;

		for (const auto& mapVar : Util::ParseJson (data, Q_FUNC_INFO).toMap () ["response"].toList ())
		{
			const auto& map = mapVar.toMap ();
			if (map.isEmpty ())
				continue;

			Media::IAudioPile::Result result;
			result.Info_.Length_ = map ["duration"].value<qint32> ();

			if (Query_.TrackLength_ > 0 && result.Info_.Length_ != Query_.TrackLength_)
			{
				qDebug () << Q_FUNC_INFO
						<< "skipping track due to track length mismatch"
						<< result.Info_.Length_
						<< Query_.TrackLength_;
				continue;
			}

			result.Info_.Artist_ = map ["artist"].toString ();
			result.Info_.Title_ = map ["title"].toString ();
			result.Source_ = map ["url"].toString ();
			results << result;
		}

		Util::ReportFutureResult (Promise_, results);
	}
}
}
