/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTIMPORT_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTIMPORT_H
#include <QMetaType>
#include <QVariantMap>

namespace LC
{
namespace Azoth
{
	/** @brief Interface for protocols supporting import.
	 *
	 * This interface should be implemented by the protocols supporting
	 * import of accounts or IM history.
	 *
	 * @sa IProtocol
	 */
	class ISupportImport
	{
	public:
		~ISupportImport () {}

		/** @brief Returns the "import-style" ID of the protocol.
		 *
		 * The following should be used, if possible:
		 * - xmpp for XMPP.
		 * - irc for IRC.
		 * - icq for ICQ.
		 *
		 * @return The "import-style" ID of the protocol.
		 */
		virtual QString GetImportProtocolID () const = 0;

		/** @brief Should try to import the account from data.
		 *
		 * The following keys are used globally, where possible:
		 * - "Name" string for account name.
		 * - "Jid" string for bare account ID (like JID in XMPP or UIN
		 *   in ICQ).
		 * - "Host" string for custom connection host, or empty.
		 * - "Port" int for custom connection port, or 0.
		 * - "Nick" string for user's nickname.
		 *
		 * @param[in] data The variant map with account settings.
		 * @return True if import was successful, false otherwise.
		 */
		virtual bool ImportAccount (const QVariantMap& data) = 0;

		/** @brief Returns the unique entry ID for given entry and account.
		 *
		 * This function is used to retrieve the unique entry ID from
		 * the given human-readable ID (hrID) and account object.
		 *
		 * The entry identified by human-readable ID may be absent in
		 * the account's contact list.
		 *
		 * @param[in] hrID The human-readable ID of the entry.
		 * @param[in] account The account object where the entry should
		 * belong.
		 * @return The would-be unique entry ID.
		 */
		virtual QString GetEntryID (const QString& hrID, QObject *account) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportImport,
		"org.Deviant.LeechCraft.Azoth.ISupportImport/1.0")

#endif
