/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mprissource.h"
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/functional.h>

namespace LC
{
namespace Xtazy
{
	const QString MPRISPrefix = "org.mpris";

	enum class MPRISVersion
	{
		V1,
		V2
	};

	namespace
	{
		MPRISVersion GetVersion (const QString& service)
		{
			return service.contains ("MediaPlayer2") ?
					MPRISVersion::V1 :
					MPRISVersion::V2;
		}
	}

	enum class PlayStatus
	{
		Playing,
		Paused,
		Stopped
	};

	struct PlayerStatus
	{
		PlayStatus PlayStatus_;
		int PlayOrder_;
		int PlayRepeat_;
		int StopOnce_;
	};
}
}

Q_DECLARE_METATYPE (LC::Xtazy::PlayerStatus);

namespace LC
{
namespace Xtazy
{
	namespace
	{
		PlayStatus GetMPRIS2PlayStatus (const QString& status)
		{
			if (status == "Playing")
				return PlayStatus::Playing;
			else if (status == "Paused")
				return PlayStatus::Paused;
			else
				return PlayStatus::Stopped;
		}
	}

	QDBusArgument& operator<< (QDBusArgument& arg, const PlayerStatus& ps)
	{
		arg.beginStructure ();
		arg << static_cast<int> (ps.PlayStatus_)
			<< ps.PlayOrder_
			<< ps.PlayRepeat_
			<< ps.StopOnce_;
		arg.endStructure ();
		return arg;
	}

	const QDBusArgument& operator>> (const QDBusArgument& arg, PlayerStatus& ps)
	{
		arg.beginStructure ();

		int rawPS = 0;
		arg >> rawPS
			>> ps.PlayOrder_
			>> ps.PlayRepeat_
			>> ps.StopOnce_;

		ps.PlayStatus_ = static_cast<PlayStatus> (rawPS);

		arg.endStructure ();
		return arg;
	}

	MPRISSource::MPRISSource (QObject *parent)
	: TuneSourceBase { "MPRIS", parent }
	, SB_ { QDBusConnection::connectToBus (QDBusConnection::SessionBus, "org.LeechCraft.Xtazy") }
	, Players_ { SB_.interface ()->registeredServiceNames ().value ().filter (MPRISPrefix) }
	{
		qDBusRegisterMetaType<PlayerStatus> ();

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
		switch (GetVersion (service))
		{
		case MPRISVersion::V1:
			SB_.connect (service,
					"/Player",
					"org.freedesktop.MediaPlayer",
					"StatusChange",
					"(iiii)",
					this,
					SLOT (handlePlayerStatusChange (PlayerStatus)));
			SB_.connect (service,
					"/Player",
					"org.freedesktop.MediaPlayer",
					"TrackChange",
					"a{sv}",
					this,
					SLOT (handleTrackChange (QVariantMap)));
			break;
		case MPRISVersion::V2:
			SB_.connect (service,
					"/org/mpris/MediaPlayer2",
					"org.freedesktop.DBus.Properties",
					"PropertiesChanged",
					this,
					SLOT (handlePropertyChange (QDBusMessage)));
			break;
		}
	}

	void MPRISSource::DisconnectFromBus (const QString& service)
	{
		switch (GetVersion (service))
		{
		case MPRISVersion::V1:
			SB_.disconnect (service,
					"/Player",
					"org.freedesktop.MediaPlayer",
					"StatusChange",
					"(iiii)",
					this,
					SLOT (handlePlayerStatusChange (PlayerStatus)));
			SB_.disconnect (service,
					"/Player",
					"org.freedesktop.MediaPlayer",
					"TrackChange",
					"a{sv}",
					this,
					SLOT (handleTrackChange (QVariantMap)));
			break;
		case MPRISVersion::V2:
			SB_.disconnect (service,
					"/org/mpris/MediaPlayer2",
					"org.freedesktop.DBus.Properties",
					"PropertiesChanged",
					this,
					SLOT (handlePropertyChange (QDBusMessage)));
			break;
		}
	}

	Media::AudioInfo MPRISSource::GetTuneMV2 (const QVariantMap& map)
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

		auto v = map.value ("Metadata");
		if (v.isValid ())
		{
			const auto& tune = GetTuneMV2 (qdbus_cast<QVariantMap> (v.value<QDBusArgument> ()));
			if (tune != Tune_)
			{
				Tune_ = tune;
				if (!Tune_.Title_.isEmpty ())
					EmitChange (Tune_);
			}
		}

		v = map.value ("PlaybackStatus");
		if (v.isValid ())
		{
			PlayerStatus status;
			status.PlayStatus_ = GetMPRIS2PlayStatus (v.toString ());
			handlePlayerStatusChange (status);
		}
	}

	void MPRISSource::handlePlayerStatusChange (const PlayerStatus& ps)
	{
		if (ps.PlayStatus_ != PlayStatus::Playing)
		{
			EmitChange ({});
			if (ps.PlayStatus_ == PlayStatus::Stopped)
				Tune_ = {};
		}
		else if (!Tune_.Title_.isEmpty ())
			EmitChange (Tune_);
	}

	void MPRISSource::handleTrackChange (const QVariantMap& map)
	{
		auto tuneMap = map;
		if (tuneMap.contains ("album"))
			tuneMap ["source"] = tuneMap.take ("album");
		if (tuneMap.contains ("time"))
			tuneMap ["length"] = tuneMap.take ("time");

		const auto& tune = FromMPRISMap (tuneMap);

		if (tune == Tune_)
			return;

		Tune_ = tune;
		if (!Tune_.Title_.isEmpty ())
			EmitChange (Tune_);
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
