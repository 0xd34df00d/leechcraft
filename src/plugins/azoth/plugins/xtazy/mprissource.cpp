/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "mprissource.h"
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Xtazy
{
	const QString MPRISPrefix = "org.mpris";

	enum MPRISVersion
	{
		MV1 = 1,
		MV2
	};

	namespace
	{
		MPRISVersion GetVersion (const QString& service)
		{
			return service.contains ("MediaPlayer2") ?
					MV1 :
					MV2;
		}
	}

	enum PlayStatus
	{
		PSPlaying = 0,
		PSPaused,
		PSStopped
	};

	namespace
	{
		PlayStatus GetMPRIS2PlayStatus (const QString& status)
		{
			if (status == "Playing")
				return PSPlaying;
			else if (status == "Paused")
				return PSPaused;
			else
				return PSStopped;
		}
	}

	QDBusArgument& operator<< (QDBusArgument& arg, const PlayerStatus& ps)
	{
		arg.beginStructure ();
		arg << ps.PlayStatus_
			<< ps.PlayOrder_
			<< ps.PlayRepeat_
			<< ps.StopOnce_;
		arg.endStructure ();
		return arg;
	}

	const QDBusArgument& operator>> (const QDBusArgument& arg, PlayerStatus& ps)
	{
		arg.beginStructure ();
		arg >> ps.PlayStatus_
			>> ps.PlayOrder_
			>> ps.PlayRepeat_
			>> ps.StopOnce_;
		arg.endStructure ();
		return arg;
	}

	MPRISSource::MPRISSource (QObject *parent)
	: TuneSourceBase (parent)
	, SB_ (QDBusConnection::sessionBus ())
	{
		setObjectName ("MPRISSource");

		qDBusRegisterMetaType<PlayerStatus> ();

		Players_ = SB_.interface ()->registeredServiceNames ()
				.value ().filter (MPRISPrefix);

		Q_FOREACH (const QString& player, Players_)
			ConnectToBus (player);

		SB_.connect ("org.freedesktop.DBus",
				"/org/freedesktop/DBus",
				"org.freedesktop.DBus",
				"NameOwnerChanged",
				this,
				SLOT (checkMPRISService (QString, QString, QString)));
	}

	MPRISSource::~MPRISSource ()
	{
		Q_FOREACH (const QString& player, Players_)
			DisconnectFromBus (player);

		SB_.disconnect ("org.freedesktop.DBus",
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
			case MV1:
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
			case MV2:
				SB_.connect (service,
						"/org/mpris/MediaPlayer2",
						"org.freedesktop.DBus.Properties",
						"PropertiesChanged",
						this,
						SLOT (handlePropertyChange (QDBusMessage)));
		}
	}

	void MPRISSource::DisconnectFromBus (const QString& service)
	{
		switch (GetVersion (service))
		{
			case MV1:
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
			case MV2:
				SB_.disconnect (service,
						"/org/mpris/MediaPlayer2",
						"org.freedesktop.DBus.Properties",
						"PropertiesChanged",
						this,
						SLOT (handlePropertyChange (QDBusMessage)));
		}
	}

	TuneSourceBase::TuneInfo_t MPRISSource::GetTuneMV2 (const QVariantMap& map)
	{
		TuneInfo_t result;
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
		return result;
	}

	void MPRISSource::handlePropertyChange (const QDBusMessage& msg)
	{
		QDBusArgument arg = msg.arguments ().at (1).value<QDBusArgument> ();
		const QVariantMap& map = qdbus_cast<QVariantMap> (arg);

		QVariant v = map.value ("Metadata");
		if (v.isValid ())
		{
			arg = v.value<QDBusArgument> ();
			TuneInfo_t tune = GetTuneMV2 (qdbus_cast<QVariantMap> (arg));
			if (tune != Tune_)
			{
				Tune_ = tune;
				if (!Tune_.isEmpty ())
					emit tuneInfoChanged (Tune_);
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

	void MPRISSource::handlePlayerStatusChange (PlayerStatus ps)
	{
		if (ps.PlayStatus_ != PSPlaying)
		{
			emit tuneInfoChanged (TuneInfo_t ());
			if (ps.PlayStatus_ == PSStopped)
				Tune_ = TuneInfo_t ();
		}
		else if (!Tune_.isEmpty ())
			emit tuneInfoChanged (Tune_);
	}

	void MPRISSource::handleTrackChange (const QVariantMap& map)
	{
		TuneInfo_t tune = map;
		if (tune.contains ("album"))
			tune ["source"] = tune.take ("album");
		if (tune.contains ("time"))
			tune ["length"] = tune.take ("time");

		if (tune == Tune_)
			return;

		Tune_ = tune;
		if (!Tune_.isEmpty ())
			emit tuneInfoChanged (Tune_);
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
}
