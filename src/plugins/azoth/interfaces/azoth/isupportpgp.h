/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTPGP_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTPGP_H
#include <QtGlobal>
#include <QtCrypto>

namespace LeechCraft
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
	public:
		virtual ~ISupportPGP () {}

		/** @brief Sets the private key for the account.
		 *
		 * @param[in] key The private key for the account.
		 */
		virtual void SetPrivateKey (const QCA::PGPKey& key) = 0;

		/** @brief Sets the public key for the given entry.
		 *
		 * @param[in] entry The entry for which to set the public key.
		 * @param[in] pubKey The public key to set.
		 */
		virtual void SetEntryKey (QObject *entry, const QCA::PGPKey& pubKey) = 0;

		/** @brief Enables or disables encryption for the given entry.
		 *
		 * If the encryption has been enabled successfully, the
		 * encryptionStateChanged() should be emitted afterwards.
		 *
		 * @param[in] entry The entry for which to toggle the encryption.
		 * @param[in] enabled Whether encryption should be enabled.
		 */
		virtual void SetEncryptionEnabled (QObject *entry, bool enabled) = 0;

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
		 */
		virtual void encryptionStateChanged (QObject *entry, bool enabled) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportPGP,
		"org.Deviant.LeechCraft.Azoth.ISupportPGP/1.0");

#endif
