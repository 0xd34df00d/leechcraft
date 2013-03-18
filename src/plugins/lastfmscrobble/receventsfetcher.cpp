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

#include "receventsfetcher.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLocale>
#include <QtDebug>
#include <QDomDocument>
#include "util.h"
#include "authenticator.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	RecEventsFetcher::RecEventsFetcher (Authenticator *auth, QNetworkAccessManager *nam, Type type, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, Type_ (type)
	{
		if (auth->IsAuthenticated ())
			request ();
		else
			connect (auth,
					SIGNAL (authenticated ()),
					this,
					SLOT (request ()));
	}

	void RecEventsFetcher::RequestEvents (QMap<QString, QString> params)
	{
		AddLanguageParam (params);
		QString method;
		switch (Type_)
		{
		case Type::Recommended:
			method = "user.getRecommendedEvents";
			break;
		case Type::Attending:
			method = "user.getEvents";
			params ["user"] = XmlSettingsManager::Instance ()
					.property ("lastfm.login").toString ();
			break;
		}

		auto reply = Request (method, NAM_, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));
	}

	void RecEventsFetcher::request ()
	{
		switch (Type_)
		{
		case Type::Recommended:
		{
			auto reply = NAM_->get (QNetworkRequest (QUrl ("http://geoip.elib.ru/cgi-bin/getdata.pl")));
			connect (reply,
					SIGNAL (finished ()),
					this,
					SLOT (handleLocationReceived ()));
			connect (reply,
					SIGNAL (error (QNetworkReply::NetworkError)),
					this,
					SLOT (handleLocationError ()));
			break;
		}
		case Type::Attending:
			RequestEvents (QMap<QString, QString> ());
			break;
		}
	}

	void RecEventsFetcher::handleLocationReceived ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply"
					<< data;
			handleLocationError ();
			return;
		}

		auto geoElem = doc.documentElement ().firstChildElement ("GeoAddr");
		const auto& lon = geoElem.firstChildElement ("Lon").text ();
		const auto& lat = geoElem.firstChildElement ("Lat").text ();
		qDebug () << Q_FUNC_INFO
				<< "fetched data:"
				<< lon
				<< lat;

		QMap<QString, QString> params;
		params ["latitude"] = lat;
		params ["longitude"] = lon;
		RequestEvents (params);
	}

	void RecEventsFetcher::handleLocationError ()
	{
		qWarning () << Q_FUNC_INFO
				<< "location fetching failed, falling back to Russia";
		sender ()->deleteLater ();

		QMap<QString, QString> params;
		params ["country"] = "Russia";
		RequestEvents (params);
	}

	namespace
	{
		QStringList GetElemsList (const QDomElement& parent, const QString& elemName)
		{
			QStringList result;
			auto elem = parent.firstChildElement (elemName);
			while (!elem.isNull ())
			{
				result << elem.text ();
				elem = elem.nextSiblingElement (elemName);
			}
			return result;
		}
	}

	void RecEventsFetcher::handleFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();
		deleteLater ();

		const auto& data = reply->readAll ();

		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply";
			return;
		}

		Media::EventInfos_t result;

		auto eventElem = doc
				.documentElement ()
				.firstChildElement ("events")
				.firstChildElement ("event");
		while (!eventElem.isNull ())
		{
			auto artistsElem = eventElem.firstChildElement ("artists");
			auto venueElem = eventElem.firstChildElement ("venue");
			auto locationElem = venueElem.firstChildElement ("location");
			auto pointElem = locationElem.firstChildElement ("point");

			Media::EventInfo info =
			{
				eventElem.firstChildElement ("id").text ().toInt (),
				eventElem.firstChildElement ("title").text (),
				QString (),
				QLocale ("en_US").toDateTime (eventElem.firstChildElement ("startDate").text (),
							"ddd, dd MMM yyyy hh:mm:ss"),
				eventElem.firstChildElement ("url").text (),
				GetImage (eventElem, "medium"),
				GetImage (eventElem, "extralarge"),
				GetElemsList (artistsElem, "artist"),
				artistsElem.firstChildElement ("headliner").text (),
				GetElemsList (eventElem.firstChildElement ("tags"), "tag"),
				eventElem.firstChildElement ("attendance").text ().toInt (),
				venueElem.firstChildElement ("name").text (),
				pointElem.firstChildElement ("lat").text ().toDouble (),
				pointElem.firstChildElement ("long").text ().toDouble (),
				locationElem.firstChildElement ("city").text (),
				locationElem.firstChildElement ("street").text (),
				true,
				Type_ == Type::Attending ?
						Media::EventAttendType::Surely :
						Media::EventAttendType::None
			};
			result << info;

			eventElem = eventElem.nextSiblingElement ("event");
		}

		emit gotRecommendedEvents (result);
	}

	void RecEventsFetcher::handleError ()
	{
		qWarning () << Q_FUNC_INFO
				<< "error fetching events"
				<< qobject_cast<QNetworkReply*> (sender ())->errorString ();

		sender ()->deleteLater ();
		deleteLater ();
	}
}
}
