/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QImage>
#include <QFuture>
#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	/** @brief Describes an entry that can have an associated avatar.
	 */
	class IHaveAvatars
	{
	protected:
		virtual ~IHaveAvatars () {}
	public:
		/** @brief The size of the avatar.
		 */
		enum class Size
		{
			/** @brief Full-size avatar.
			 */
			Full,

			/** @brief Thumbnail avatar, possibly cropped.
			 */
			Thumbnail
		};

		/** @brief Requests the avatar of the given \em size.
		 *
		 * This function should schedule fetching the avatar and return a
		 * future that will become ready once the avatar is fetched.
		 *
		 * If there is no avatar, a null QImage should be returned.
		 *
		 * No cached copies should be returned, the avatar information
		 * should be rerequested from scratch (hence <em>refrech</em> in
		 * the method name).
		 *
		 * If \em size is not supported, an empty, cancelled future should
		 * be returned.
		 *
		 * @param[in] size The required size of the avatar.
		 * @return A future providing the avatar's image.
		 */
		virtual QFuture<QImage> RefreshAvatar (Size size) = 0;

		/** @brief Returns whether this exact entry has any avatar.
		 *
		 * @return Whether there is an avatar.
		 */
		virtual bool HasAvatar () const = 0;

		/** @brief Whether this entry has an avatar of the given \em size.
		 *
		 * @param[in] size The desired size of the avatar.
		 * @return Whether the entry has this size.
		 */
		virtual bool SupportsSize (Size size) const = 0;
	protected:
		/** @brief Notifies that the avatar of the entry has been changed.
		 *
		 * This signal should be emitted when the entry detects that the
		 * avatar has been changed.
		 *
		 * If possible, the new avatar should \em not be fetched unless
		 * RefreshAvatar() is called.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] thisObject The <code>this</code> pointer.
		 */
		virtual void avatarChanged (QObject *thisObject) = 0;
	};

	/** @brief Defines a hashing function for avatar sizes.
	 *
	 * @param[in] size The size of the avatar.
	 * @return The hash of the size.
	 */
	inline uint qHash (IHaveAvatars::Size size)
	{
		return static_cast<uint> (size);
	}
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveAvatars,
		"org.LeechCraft.Azoth.IHaveAvatars/1.0")
