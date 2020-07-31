/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QByteArray>
#include "headers.h"
#include "conversions.h"

class QString;

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	struct Packet;

	class PacketFactory
	{
		quint32 Seq_;
	public:
		PacketFactory ();

		Packet Hello ();
		Packet Ping ();
		Packet Login (const QString& login, const QString& pass,
				quint32 state, const QString& status, const QString& ua);
		Packet SetStatus (quint32 state, const QString& status);

		Packet RequestInfo (const QString& id);
		Packet Message (MsgFlags flags, const QString& to, const QString& msg);
		Packet MessageAck (const QString& from, quint32 msgId);
		Packet OfflineMessageAck (const UIDL& id);
		Packet Microblog (BlogStatus st, const QString& text);
		Packet SMS (const QString& to, const QString& text);

		Packet AddGroup (const QString& name, int numGroups);
		Packet AddContact (ContactOpFlags flags, quint32 group,
				const QString& email, const QString& name);
		Packet ModifyContact (quint32 cid, ContactOpFlags flags,
				quint32 group, const QString& email, const QString& name, const QString& phone);
		Packet RemoveContact (quint32 id,
				const QString& email, const QString& name);

		Packet Authorize (const QString&);
		Packet RequestKey ();
	};
}
}
}
}
