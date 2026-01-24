/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QString>

namespace LC::Azoth
{
	/** @brief Describes a message to be sent,
	 * as formed by the user and before it has been sent to the wire.
	 */
	struct OutgoingMessage
	{
		std::optional<QString> Variant_ {};
		QString Body_;
		std::optional<QString> RichTextBody_ {};

		bool AllowServerArchival_ = true;
		bool Hidden_ = false;
	};

	struct InjectedMessage
	{
		std::optional<QString> Variant_ {};
		QDateTime TS_ = QDateTime::currentDateTime ();
		QString Body_;
		std::optional<QString> RichTextBody_ {};

		enum class Direction : uint8_t
		{
			In,
			Out,
		};

		struct Chat { Direction Dir_; };
		struct Service {};
		using Kind = std::variant<Chat, Service>;
		Kind Kind_;

		static InjectedMessage FromOutgoing (const OutgoingMessage& msg)
		{
			return
			{
				.Body_ = msg.Body_,
				.RichTextBody_ = msg.RichTextBody_,
				.Kind_ = Chat { Direction::Out },
			};
		}
	};
}
