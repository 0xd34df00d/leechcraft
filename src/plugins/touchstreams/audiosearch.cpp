/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include "authmanager.h"

namespace LeechCraft
{
namespace TouchStreams
{
	AudioSearch::AudioSearch (ICoreProxy_ptr proxy,
			const QString& query, AuthManager *mgr, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, AuthMgr_ (mgr)
	, Query_ (query)
	{
		connect (AuthMgr_,
				SIGNAL (gotAuthKey (QString)),
				this,
				SLOT (handleGotAuthKey (QString)));
		AuthMgr_->GetAuthKey ();
	}

	QObject* AudioSearch::GetObject ()
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
		url.addQueryItem ("q", Query_);

		auto reply = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (url));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotReply ()));
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
				result.Info_.Artist_ = QString::fromUtf8 (sub.get<std::string> ("artist").c_str ());
				result.Info_.Title_ = QString::fromUtf8 (sub.get<std::string> ("title").c_str ());
				result.Info_.Length_ = sub.get<qint32> ("duration");
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
	}
}
}
