/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IURIHANDLER_H
#define PLUGINS_AZOTH_INTERFACES_IURIHANDLER_H
#include <QUrl>
#include <QObject>

namespace LC
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

Q_DECLARE_INTERFACE (LC::Azoth::IURIHandler,
		"org.Deviant.LeechCraft.Azoth.IURIHandler/1.0")

#endif
