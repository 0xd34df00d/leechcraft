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
	/** @brief Interface for accounts supporting audio/video calls.
	 *
	 * This interface should be implemented by IAccount instances wishing
	 * to advertise the support for audio or video calls to the rest of
	 * Azoth.
	 *
	 * @sa IAccount
	 * @sa IMediaCall
	 */
	class ISupportMediaCalls
	{
	public:
		virtual ~ISupportMediaCalls () {}

		/** @brief Describes supported media call features.
		 */
		enum MediaCallFeature
		{
			/** @brief No particular features.
			 */
			MCFNoFeatures,

			/** @brief The account supports audio calls.
			 */
			MCFSupportsAudioCalls = 0x01,

			/** @brief The accounts supports video calls.
			 */
			MCFSupportsVideoCalls = 0x02
		};

		Q_DECLARE_FLAGS (MediaCallFeatures, MediaCallFeature)

		/** @brief Returns the media features supported by this account.
		 *
		 * @return The account media call features.
		 */
		virtual MediaCallFeatures GetMediaCallFeatures () const = 0;

		/** @brief Tries to call a contact list entry.
		 *
		 * The entry is identified by its \em id, which is the ID returned
		 * by ICLEntry::GetEntryID().
		 *
		 * If the corresponding protocol supports multiple variants per
		 * entry, the \em variant parameter specifies which variant (from
		 * the ones returned from ICLEntry::Variants()) of the entry
		 * should be called.
		 *
		 * Returns either a IMediaCall object or a nullptr if the call
		 * initialization failed.
		 *
		 * @note If this method returns a valid IMediaCall object, it
		 * should also be advertised via the called() signal.
		 *
		 * @param[in] id The ID of the entry to call (as in ICLEntry::GetEntryID()).
		 * @param[in] variant The variant of the entry to call to.
		 * @return The call object implementing IMediaCall, or a nullptr
		 * if it is impossible to make the call.
		 */
		virtual QObject* Call (const QString& id, const QString& variant) = 0;
	protected:
		/** @brief Emitted when a new call is established.
		 *
		 * This signal should be emitted whenever a new call is
		 * established, either an incoming call or an outgoing call
		 * initiated via the Call() method.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] callObj The object representing the call and
		 * implementing IMediaCall.
		 */
		virtual void called (QObject *callObj) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportMediaCalls,
		"org.Deviant.LeechCraft.Azoth.ISupportMediaCalls/1.0")
