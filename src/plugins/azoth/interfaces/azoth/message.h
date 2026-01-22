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
		QDateTime TS_ = QDateTime::currentDateTime ();
		QString Body_;
		std::optional<QString> RichTextBody_ {};

		/** @brief Represents the direction of the message.
		 */
		enum class Direction : uint8_t
		{
			/** @brief The message is from the remote party to us.
			 */
			In,

			/** @brief The message is from us to the remote party.
			 */
			Out,
		};

		struct ChatMessage { Direction Dir_; };
		struct ServiceMessage {};
		using Kind = std::variant<ChatMessage, ServiceMessage>;
		Kind Kind_;

		static InjectedMessage FromOutgoing (const OutgoingMessage& msg)
		{
			return
			{
				.Body_ = msg.Body_,
				.RichTextBody_ = msg.RichTextBody_,
				.Kind_ = ChatMessage { Direction::Out },
			};
		}
	};
}
