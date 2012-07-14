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

#ifndef PLUGINS_AZOTH_INTERFACES_IURIHANDLER_H
#define PLUGINS_AZOTH_INTERFACES_IURIHANDLER_H
#include <QUrl>
#include <QObject>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief This interface is for protocols that may handle URIs and
	 * corresponding actions are dependent on an exact account.
	 * 
	 * The protocols that implement this interface are queried by the
	 * Azoth Core whenever an URI should be checked if it could be
	 * handled. If a protocol defines that it supports a given URI by
	 * returning true from SupportsURI() method, then Core queries for
	 * the list of accounts of this protocol and asks user to select one
	 * that should be used to handle the URI.
	 * 
	 * If several different protocols show that they support a given
	 * URI, then accounts from all of them would be suggested to the
	 * user.
	 */
	class IURIHandler
	{
	public:
		virtual ~IURIHandler () {}
		
		/** @brief Queries whether the given URI is supported.
		 * 
		 * @param[in] uri The URI to query.
		 * 
		 * @return Whether this URI could be handled by an account of
		 * this protocol.
		 */
		virtual bool SupportsURI (const QUrl& uri) const = 0;
		
		/** @brief Asks to handle the given URI by the given account.
		 * 
		 * The account is selected by the user from the list of accounts
		 * of this protocol if SupportsURI() returned true for the given
		 * URI.
		 * 
		 * @param[in] uri The URI to handle.
		 * @param[in] asAccount The account to use to handle this URI.
		 */
		virtual void HandleURI (const QUrl& uri, QObject *asAccount) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IURIHandler,
		"org.Deviant.LeechCraft.Azoth.IURIHandler/1.0");

#endif
