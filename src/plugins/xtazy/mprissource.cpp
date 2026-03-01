/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mprissource.h"
#include <QDBusConnectionInterface>

namespace LC
{
namespace Xtazy
{
	const QString MPRISPrefix = "org.mpris";

	enum class PlayStatus
	{
		Playing,
		Paused,
		Stopped
	};

	namespace
	{
		PlayStatus GetMPRIS2PlayStatus (const QString& status)
		{
			if (status == "Playing")
				return PlayStatus::Playing;
			if (status == "Paused")
				return PlayStatus::Paused;
			return PlayStatus::Stopped;
		}
	}

	MPRISSource::MPRISSource (QObject *parent)
	: TuneSourceBase { "MPRIS", parent }
	, SB_ { QDBusConnection::connectToBus (QDBusConnection::SessionBus, "org.LeechCraft.Xtazy") }
	, Players_ { SB_.interface ()->registeredServiceNames ().value ().filter (MPRISPrefix) }
	{
		for (const auto& player : Players_)
			ConnectToBus (player);

		SB_.connect ("org.freedesktop.DBus",
				"/org/freedesktop/DBus",
				"org.freedesktop.DBus",
				"NameOwnerChanged",
				this,
				SLOT (checkMPRISService (QString, QString, QString)));
	}

	void MPRISSource::ConnectToBus (const QString& service)
	{
		SB_.connect (service,
				"/org/mpris/MediaPlayer2",
				"org.freedesktop.DBus.Properties",
				"PropertiesChanged",
				this,
				SLOT (handlePropertyChange (QDBusMessage)));
	}

	void MPRISSource::DisconnectFromBus (const QString& service)
	{
		SB_.disconnect (service,
				"/org/mpris/MediaPlayer2",
				"org.freedesktop.DBus.Properties",
				"PropertiesChanged",
				this,
				SLOT (handlePropertyChange (QDBusMessage)));
	}

	Media::AudioInfo MPRISSource::GetTuneInfo (const QVariantMap& map)
	{
		QVariantMap result;
		if (map.contains ("xesam:title"))
			result ["title"] = map ["xesam:title"];
		if (map.contains ("xesam:artist"))
			result ["artist"] = map ["xesam:artist"];
		if (map.contains ("xesam:album"))
			result ["source"] = map ["xesam:album"];
		if (map.contains ("xesam:trackNumber"))
			result ["track"] = map ["xesam:trackNumber"];
		if (map.contains ("xesam:length"))
			result ["length"] = map ["xesam:length"].toLongLong () / 1000000;
		return FromMPRISMap (result);
	}

	void MPRISSource::handlePropertyChange (const QDBusMessage& msg)
	{
		const auto& arg = msg.arguments ().at (1).value<QDBusArgument> ();
		const auto& map = qdbus_cast<QVariantMap> (arg);

		if (const auto metadataVar = map.value ("Metadata");
			metadataVar.isValid ())
			if (const auto& tune = GetTuneInfo (qdbus_cast<QVariantMap> (metadataVar.value<QDBusArgument> ()));
				tune != Tune_)
			{
				Tune_ = tune;
				if (!Tune_.Title_.isEmpty ())
					EmitChange (Tune_);
			}

		if (const auto statusVar = map.value ("PlaybackStatus");
			statusVar.isValid ())
		{
			if (const auto status = GetMPRIS2PlayStatus (statusVar.toString ());
				status != PlayStatus::Playing)
			{
				EmitChange ({});
				if (status == PlayStatus::Stopped)
					Tune_ = {};
			}
			else if (!Tune_.Title_.isEmpty ())
				EmitChange (Tune_);
		}
	}

	void MPRISSource::checkMPRISService (QString name,
			QString, QString newOwner)
	{
		if (!name.startsWith (MPRISPrefix))
			return;

		const int playerIdx = Players_.indexOf (name);
		if (playerIdx == -1)
		{
			if (!newOwner.isEmpty ())
			{
				Players_ << name;
				ConnectToBus (name);
			}
		}
		else if (newOwner.isEmpty ())
		{
			DisconnectFromBus (name);
			Players_.removeAt (playerIdx);
		}
	}
}
}
