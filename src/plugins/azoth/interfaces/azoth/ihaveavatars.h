/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/sll/either.h>
#include <util/threads/coro/taskfwd.h>

namespace LC::Azoth
{
	/** @brief Describes an entry that can have an associated avatar.
	 */
	class IHaveAvatars
	{
	protected:
		virtual ~IHaveAvatars () {}
	public:
		struct FetchError {};

		using FetchResult_t = Util::Either<FetchError, QByteArray>;

		/** @brief Requests the contact avatar.
		 *
		 * If there is no avatar, an empty `QByteArray` should be returned.
		 *
		 * This function does not need to cache the avatar.
		 *
		 * @return Pending avatar image contents.
		 */
		virtual Util::ContextTask<FetchResult_t> FetchAvatar () = 0;

		/** @brief Returns whether this exact entry has any avatar.
		 *
		 * @return Whether there is an avatar.
		 */
		virtual bool HasAvatar () const = 0;
	protected:
		/** @brief Notifies that the avatar of the entry has been changed.
		 *
		 * This signal should be emitted when the entry detects that the
		 * avatar has been changed.
		 *
		 * If possible, the new avatar should \em not be fetched unless
		 * `FetchAvatar()` is called.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] thisObject The <code>this</code> pointer.
		 */
		virtual void avatarChanged (QObject *thisObject) = 0;
	};
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveAvatars,
		"org.LeechCraft.Azoth.IHaveAvatars/1.0")
