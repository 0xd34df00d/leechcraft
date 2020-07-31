/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>
#include <QtCrypto>
#include "gpgexceptions.h"

namespace LC
{
namespace Azoth
{
	/** @brief Interface for accounts supporting PGP encryption.
	 *
	 * This interface should be implemented by accounts which support
	 * PGP encryption, only if ENABLE_CRYPT compile-time option
	 * is enabled.
	 *
	 * @sa IAccount
	 */
	class ISupportPGP
	{
	protected:
		virtual ~ISupportPGP () = default;
	public:
		/** @brief Sets the private key for the account.
		 *
		 * @param[in] key The private key for the account.
		 */
		virtual void SetPrivateKey (const QCA::PGPKey& key) = 0;

		/** @brief Returns the private key for the account, if any.
		 *
		 * @return Private key for the account, or an empty key if no
		 * key has been set.
		 */
		virtual QCA::PGPKey GetPrivateKey () const = 0;

		/** @brief Sets the public key for the given entry.
		 *
		 * @param[in] entry The entry for which to set the public key.
		 * @param[in] pubKey The public key to set.
		 */
		virtual void SetEntryKey (QObject *entry, const QCA::PGPKey& pubKey) = 0;

		/** @brief Returns the public key for the given entry, if any.
		 *
		 * If there is no public key for the entry, an empty key should
		 * be returned.
		 *
		 * @param[in] entry The entry for which to return the public key.
		 * @return The public key for the given entry, if any.
		 */
		virtual QCA::PGPKey GetEntryKey (QObject *entry) const = 0;

		/** @brief Enables or disables encryption for the \em entry.
		 *
		 * If the encryption status has been changed successfully, the
		 * encryptionStateChanged() should be emitted afterwards.
		 *
		 * @param[in] entry The entry for which to toggle the encryption.
		 * @param[in] enabled Whether encryption should be enabled.
		 *
		 * @sa encryptionStateChanged()
		 */
		virtual GPGExceptions::MaybeException_t SetEncryptionEnabled (QObject *entry, bool enabled) = 0;

		/** @brief Checks if the encryption is enabled for the \em entry.
		 *
		 * @param[in] entry The entry for which to query the encryption
		 * status.
		 * @return Whether the encryption has been enabled successfully
		 * for the \em entry.
		 *
		 * @sa SetEncryptionEnabled()
		 * @sa encryptionStateChanged()
		 */
		virtual bool IsEncryptionEnabled (QObject *entry) const = 0;
	protected:
		/** @brief Notifies whether signature has been verified for the
		 * given entry.
		 *
		 * This signal should be emitted whenever protocol receives a
		 * PGP signature from the given entry, no matter if that
		 * signature is valid or not. If the signature is valid,
		 * successful should be set to true, otherwise it should be
		 * false, but the signal still has to be emitted.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] entry The entry for which signature has been
		 * attempted to be verified.
		 * @param[out] successful Whether verification was successful.
		 */
		virtual void signatureVerified (QObject *entry, bool successful) = 0;

		/** @brief Notifies that encryption state has changed for the
		 * given entry.
		 *
		 * This signal should be emitted whenever encryption state
		 * changes for the given entry, even as the result of
		 * SetEncryptionEnabled() call.
		 *
		 * @param[out] entry The entry for which encryption state has
		 * changed.
		 * @param[out] enabled Whether encryption has been enabled or
		 * disabled.
		 *
		 * @sa SetEncryptionEnabled()
		 * @sa IsEncryptionEnabled()
		 */
		virtual void encryptionStateChanged (QObject *entry, bool enabled) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportPGP,
		"org.Deviant.LeechCraft.Azoth.ISupportPGP/1.0")
