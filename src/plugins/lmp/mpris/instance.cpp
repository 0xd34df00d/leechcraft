/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "instance.h"
#include <QDBusConnection>
#include "../player.h"
#include "mediaplayer2adaptor.h"
#include "playeradaptor.h"
#include "fdopropsadaptor.h"

namespace LC
{
namespace LMP
{
namespace MPRIS
{
	Instance::Instance (QObject *tab, Player *player)
	: QObject (tab)
	{
		auto fdo = new FDOPropsAdaptor (player);
		new MediaPlayer2Adaptor (tab, player);
		new PlayerAdaptor (fdo, player);

		QDBusConnection::sessionBus ().registerService ("org.mpris.MediaPlayer2.LMP");
		QDBusConnection::sessionBus ().registerObject ("/org/mpris/MediaPlayer2", player);
	}
}
}
}
