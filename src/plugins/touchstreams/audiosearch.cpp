/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "audiosearch.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <util/queuemanager.h>
#include "authmanager.h"

namespace LeechCraft
{
namespace TouchStreams
{
	AudioSearch::AudioSearch (ICoreProxy_ptr proxy,
			const Media::AudioSearchRequest& query, AuthManager *mgr, Util::QueueManager *queue, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Queue_ (queue)
	, AuthMgr_ (mgr)
	, Query_ (query)
	{
		connect (AuthMgr_,
				SIGNAL (gotAuthKey (QString)),
				this,
				SLOT (handleGotAuthKey (QString)));
		AuthMgr_->GetAuthKey ();
	}

	QObject* AudioSearch::GetQObject ()
	{
		return this;
	}

	QList<Media::IPendingAudioSearch::Result> AudioSearch::GetResults () const
	{
		return Result_;
	}

	void AudioSearch::handleGotAuthKey (const QString& key)
	{
		QUrl url ("https://api.vk.com/method/audio.search");
		url.addQueryItem ("access_token", key);
		url.addQueryItem ("q", Query_.FreeForm_);

		Queue_->Schedule ([this, url] () -> void
			{
				auto reply = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (url));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotReply ()));
				connect (reply,
						SIGNAL (error (QNetworkReply::NetworkError)),
						this,
						SLOT (handleError ()));
			}, this);
	}

	void AudioSearch::handleGotReply ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		std::istringstream istr (data.constData ());
		boost::property_tree::ptree pt;
		boost::property_tree::read_json (istr, pt);

		for (const auto& v : pt.get_child ("response"))
		{
			const auto& sub = v.second;
			if (sub.empty ())
				continue;

			Media::IPendingAudioSearch::Result result;
			try
			{
				result.Info_.Length_ = sub.get<qint32> ("duration");

				if (Query_.TrackLength_ > 0 && result.Info_.Length_ != Query_.TrackLength_)
				{
					qDebug () << Q_FUNC_INFO
							<< "skipping track due to track length mismatch"
							<< result.Info_.Length_
							<< Query_.TrackLength_;
					continue;
				}

				result.Info_.Artist_ = QString::fromUtf8 (sub.get<std::string> ("artist").c_str ());
				result.Info_.Title_ = QString::fromUtf8 (sub.get<std::string> ("title").c_str ());
				result.Source_ = QUrl::fromEncoded (sub.get<std::string> ("url").c_str ());
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to get props"
						<< e.what ();
				continue;
			}
			Result_ << result;
		}

		emit ready ();
		emit deleteLater ();
	}

	void AudioSearch::handleError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();
		emit error ();
		deleteLater ();
	}
}
}
