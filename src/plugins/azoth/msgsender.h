/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include <interfaces/core/ihookproxy.h>
#include "interfaces/azoth/imessage.h"

namespace LC
{
namespace Azoth
{
	class MsgSender : public QObject
	{
		Q_OBJECT
	public:
		MsgSender (ICLEntry *e, IMessage::Type type,
				QString text, QString variant = {}, QString richText = {});
	signals:
		void hookMessageWillCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant);
		void hookMessageCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *message);
	};
}
}
