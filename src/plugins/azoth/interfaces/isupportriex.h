/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTRIEX_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTRIEX_H
#include <QFlags>
#include <QMetaType>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface representing Roster Item Exchange-like things.
	 * 
	 * This interface should be implemented by account objects that
	 * support exchanging contact list items between different users.
	 * 
	 * This interface is modeled after XEP-0144.
	 */
	class ISupportRIEX
	{
	public:
		virtual ~ISupportRIEX () {}

		/** @brief This signal should be emitted when adding a contact
		 * should be suggested to the user.
		 * 
		 * For XMPP protocol, this signal may be emitted when new
		 * contacts are pushed to the roster via Roster Exchange XEP.
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] id The ID of the contact.
		 * @param[out] nick The suggested nick of the contact.
		 * @param[out] groups The suggested groups of the contact.
		 */
		virtual void addContactSuggested (const QString& id,
				const QString& nick, const QStringList& groups) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportRIEX,
		"org.Deviant.LeechCraft.Azoth.ISupportRIEX/1.0");

#endif
