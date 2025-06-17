/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "userlocation.h"
#include <QUrl>
#include <QDateTime>
#include <QDomElement>
#include <QtDebug>
#include <QXmppElement.h>
#include <util/sll/qtutil.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsLocationNode = "http://jabber.org/protocol/geoloc";

	QString UserLocation::GetNodeString ()
	{
		return NsLocationNode;
	}

	QXmppElement UserLocation::ToXML () const
	{
		QXmppElement geoloc;
		geoloc.setTagName ("geoloc");
		geoloc.setAttribute ("xmlns", NsLocationNode);

		for (const auto& [key, val] : Util::Stlize (Info_))
		{
			QXmppElement elem;
			elem.setTagName (key);

			if (val.typeId () == QMetaType::QDateTime)
				elem.setValue (val.toDateTime ().toString (Qt::ISODate));
			else if (val.typeId () == QMetaType::QUrl)
				elem.setValue (val.toUrl ().toEncoded ());
			else
				elem.setValue (val.toString ());

			geoloc.appendChild (elem);
		}

		QXmppElement result;
		result.setTagName ("item");
		result.appendChild (geoloc);
		return result;
	}

	namespace
	{
		template<typename T>
		struct Converter;

		template<>
		struct Converter<QString>
		{
			QString operator() (const QString& str) const
			{
				return str;
			}
		};

		template<>
		struct Converter<double>
		{
			double operator() (const QString& str) const
			{
				return str.toDouble ();
			}
		};

		template<>
		struct Converter<QUrl>
		{
			QUrl operator() (const QString& str) const
			{
				return QUrl::fromEncoded (str.toUtf8 ());
			}
		};

		template<>
		struct Converter<QDateTime>
		{
			QDateTime operator() (const QString& str) const
			{
				return QDateTime::fromString (str, Qt::ISODate);
			}
		};

		template<typename T>
		struct ParseElem
		{
			const QDomElement& R_;
			GeolocationInfo_t& I_;

			ParseElem (const QDomElement& root, GeolocationInfo_t& info)
			: R_ (root)
			, I_ (info)
			{
			}

			ParseElem operator() (const char *elemName)
			{
				const QDomElement& child = R_.firstChildElement (elemName);
				if (!child.isNull ())
					I_ [elemName] = Converter<T> () (child.text ());
				return *this;
			}

			template<typename U>
			ParseElem<U> operator() (const U&)
			{
				return ParseElem<U> (R_, I_);
			}
		};
	}

	void UserLocation::Parse (const QDomElement& elem)
	{
		Info_.clear ();

		const QDomElement& geolocElem = elem.firstChildElement ("geoloc");
		if (geolocElem.namespaceURI () != NsLocationNode)
			return;

		ParseElem<double> (geolocElem, Info_)
			("accuracy")
			("alt")
			("bearing")
			("lat")
			("lon")
			("speed")
			(QString ())
			("area")
			("building")
			("country")
			("countrycode")
			("datum")
			("description")
			("floor")
			("locality")
			("postalcode")
			("region")
			("room")
			("street")
			("text")
			(QDateTime ())
			("timestamp")
			(QUrl ())
			("uri");
	}

	QString UserLocation::Node () const
	{
		return GetNodeString ();
	}

	PEPEventBase* UserLocation::Clone () const
	{
		return new UserLocation (*this);
	}

	GeolocationInfo_t UserLocation::GetInfo () const
	{
		return Info_;
	}

	void UserLocation::SetInfo (const GeolocationInfo_t& info)
	{
		Info_ = info;
	}
}
}
}
