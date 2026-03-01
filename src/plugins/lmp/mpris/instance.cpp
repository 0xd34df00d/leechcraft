/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "instance.h"
#include <QDBusConnection>
#include <util/sll/qtutil.h>
#include "../player.h"
#include "mediaplayer2adaptor.h"
#include "playeradaptor.h"

namespace LC::LMP::MPRIS
{
	namespace
	{
		QString GetServiceName ()
		{
			return "org.mpris.MediaPlayer2."_ql + qEnvironmentVariable ("LC_LMP_MPRIS_NAME_OVERRIDE", "LMP"_qs);
		}
	}

	Instance::Instance (Player *player, QObject *parent)
	: QObject { parent }
	{
		const auto mp2 = new MediaPlayer2Adaptor { player };
		connect (mp2,
				&MediaPlayer2Adaptor::raiseRequested,
				this,
				&Instance::raiseRequested);

		new PlayerAdaptor { player };

		QDBusConnection::sessionBus ().registerService (GetServiceName ());
		QDBusConnection::sessionBus ().registerObject ("/org/mpris/MediaPlayer2"_qs, player);
	}
}
