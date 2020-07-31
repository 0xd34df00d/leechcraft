/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	/** @brief Interface for entries that can modify messages bodies.
	 *
	 * This interface is to be implemented by entries (implementing
	 * ICLEntry) that can modify the bodies of the messages after the
	 * messages were received and handled by Azoth.
	 *
	 * When the body of the message is to be changed, the performJS()
	 * should be emitted with the corresponding JavaScript code that
	 * changes the message in the opened chat tabs. IMessage::GetBody()
	 * should also return the new message body after this signal.
	 *
	 * For example, a social network protocol plugin may emit an
	 * ICLEntry::gotMessage() signal when the message is received, but
	 * request additional information from the social network about the
	 * attachments, emitting IUpdatableChatEntry::performJS() after that
	 * info is fetched.
	 *
	 * @sa ICLEntry
	 */
	class IUpdatableChatEntry
	{
	public:
		virtual ~IUpdatableChatEntry () {}
	protected:
		/** @brief Emitted when a message body is to be changed.
		 *
		 * This signal should be emitted by entries when the contents of
		 * one or more messages should be changed. The corresponding
		 * \em js code is executed in an opened chat tab corresponding to
		 * the entry (if any).
		 *
		 * Typically, a message will contain a <code>div</code> or a
		 * <code>span</code> element with an unique  <code>id</code>
		 * attribute which is to be manipulated by the \em js code.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] js The code to execute in a chat tab corresponding
		 * to this entry, if any.
		 */
		virtual void performJS (const QString& js) = 0;
	};
}
}
Q_DECLARE_INTERFACE (LC::Azoth::IUpdatableChatEntry,
		"org.Deviant.LeechCraft.Azoth.IUpdatableChatEntry/1.0")
