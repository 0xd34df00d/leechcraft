/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mediaplayer2adaptor.h"
#include <QMetaObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <interfaces/imwproxy.h>
#include "../core.h"
#include "../player.h"

namespace LC
{
namespace LMP
{
namespace MPRIS
{
	MediaPlayer2Adaptor::MediaPlayer2Adaptor (QObject *tab, Player *player)
	: QDBusAbstractAdaptor (player)
	, Tab_ (tab)
	{
		setAutoRelaySignals (true);
	}

	bool MediaPlayer2Adaptor::GetCanQuit () const
	{
		return false;
	}

	bool MediaPlayer2Adaptor::GetCanSetFullscreen () const
	{
		return false;
	}

	QString MediaPlayer2Adaptor::GetDesktopEntry () const
	{
		return "leechcraft";
	}

	bool MediaPlayer2Adaptor::GetHasTrackList () const
	{
		return true;
	}

	QString MediaPlayer2Adaptor::GetIdentity () const
	{
		return "LMP";
	}

	QStringList MediaPlayer2Adaptor::GetSupportedMimeTypes () const
	{
		return
		{
			"application/ogg",
			"audio/mp4",
			"audio/mpeg",
			"audio/ogg",
			"audio/vorbis",
			"audio/x-ms-wma",
			"audio/vnd.rn-realaudio",
			"audio/vnd.wave",
			"audio/wav",
			"audio/webm",
			"audio/x-aiff",
			"audio/x-mpegurl",
			"audio/x-wav"
		};
	}

	QStringList MediaPlayer2Adaptor::GetSupportedUriSchemes () const
	{
		return { "file", "http", "https" };
	}

	void MediaPlayer2Adaptor::Quit ()
	{
	}

	void MediaPlayer2Adaptor::Raise ()
	{
		QMetaObject::invokeMethod (Tab_, "fullRaiseRequested");
	}
}
}
}
