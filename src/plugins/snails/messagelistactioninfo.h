/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QString>
#include <QIcon>
#include <QList>
#include <QMetaType>
#include <util/sll/bitflags.h>

namespace LC
{
namespace Snails
{
	struct MessageInfo;

	enum class MessageListActionFlag
	{
		None 			= 0b00,
		AlwaysPresent 	= 0b01
	};

	struct MessageListActionInfo
	{
		QString Name_;
		QString Description_;
		QIcon Icon_;
		QColor ReprColor_;
		Util::BitFlags<MessageListActionFlag> Flags_;

		std::function<void (MessageInfo)> Handler_;

		QList<MessageListActionInfo> Children_;
	};
}
}

DECLARE_BIT_FLAGS (LC::Snails::MessageListActionFlag)

Q_DECLARE_METATYPE (LC::Snails::MessageListActionInfo)
Q_DECLARE_METATYPE (QList<LC::Snails::MessageListActionInfo>)
