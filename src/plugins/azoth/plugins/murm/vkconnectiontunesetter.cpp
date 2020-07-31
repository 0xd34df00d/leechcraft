/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkconnectiontunesetter.h"
#include <QStringList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <util/sll/urloperator.h>
#include <util/sll/parsejson.h>
#include "vkconnection.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	VkConnectionTuneSetter::VkConnectionTuneSetter (VkConnection *conn, const ICoreProxy_ptr& proxy)
	: QObject { conn }
	, Conn_ { conn }
	, Proxy_ { proxy }
	{
	}

	void VkConnectionTuneSetter::SetTune (const QVariantMap& tune)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		const auto& query = (tune ["artist"].toString () + " " + tune ["title"].toString ()).simplified ();
		if (query.isEmpty () || query == LastQuery_)
			return;

		LastQuery_ = query;
		Conn_->QueueRequest ([=] (const QString& key, const VkConnection::UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/audio.search");
				Util::UrlOperator { url }
						("access_token", key)
						("q", query);

				VkConnection::AddParams (url, params);

				auto reply = nam->get (QNetworkRequest (url));
				new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[this, reply, tune]
					{
						HandleAudioSearchResults (reply, tune);
						reply->deleteLater ();
					},
					reply,
					SIGNAL (finished ()),
					this
				};
				return reply;
			});
	}

	namespace
	{
		QString SelectTopId (const QVariantList& audios, const QVariantMap& tune)
		{
			QMap<int, QString> scoredIds;

			const int artistMatch = 10;
			const int fullTitleMatch = 20;
			const auto lengthPenalty = [] (int diff) { return diff * diff; };

			for (const auto& audioVar : audios)
			{
				const auto& map = audioVar.toMap ();

				int score = 0;

				auto checkStrMatch = [&score, &map, &tune] (const QString& mapKey,
						const QString& tuneKey, int matchScore)
				{
					if (map [mapKey].toString ().toLower () == tune [tuneKey].toString ().toLower ())
						score -= matchScore;
				};

				checkStrMatch ("artist", "artist", artistMatch);
				checkStrMatch ("title", "title", fullTitleMatch);

				if (const auto length = tune.value ("length").toLongLong ())
					score += lengthPenalty (length - map ["duration"].toLongLong ());

				scoredIds [score] = map ["owner_id"].toString () + "_" + map ["id"].toString ();
			}

			return scoredIds.begin ().value ();
		}
	}

	void VkConnectionTuneSetter::HandleAudioSearchResults (QNetworkReply *reply, const QVariantMap& tune)
	{
		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		const auto& audios = data.toMap () ["response"].toMap () ["items"].toList ();

		if (audios.isEmpty ())
		{
			PublishDumbStatus (tune);
			return;
		}

		const auto& topId = SelectTopId (audios, tune);

		auto nam = Proxy_->GetNetworkAccessManager ();
		Conn_->QueueRequest ([topId, nam] (const QString& key, const VkConnection::UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/audio.setBroadcast");
				Util::UrlOperator { url }
						("access_token", key)
						("audio", topId);

				VkConnection::AddParams (url, params);

				auto reply = nam->get (QNetworkRequest (url));
				connect (reply,
						SIGNAL (finished ()),
						reply,
						SLOT (deleteLater ()));
				return reply;
			});
	}

	void VkConnectionTuneSetter::PublishDumbStatus (const QVariantMap& tune)
	{
		QStringList fields
		{
			tune ["artist"].toString (),
			tune ["source"].toString (),
			tune ["title"].toString ()
		};
		fields.removeAll ({});
		Conn_->SetStatus (fields.join (QString::fromUtf8 (" — ")));
	}
}
}
}
