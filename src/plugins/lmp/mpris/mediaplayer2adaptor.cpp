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
#include "../playertab.h"

namespace LeechCraft
{
namespace LMP
{
namespace MPRIS
{
	MediaPlayer2Adaptor::MediaPlayer2Adaptor (PlayerTab *tab)
	: QDBusAbstractAdaptor (tab)
	, Tab_ (tab)
	{
		setAutoRelaySignals (true);
	}

	MediaPlayer2Adaptor::~MediaPlayer2Adaptor ()
	{
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
		return { "application/ogg", "audio/mpeg" };
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

