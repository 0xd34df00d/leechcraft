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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVESEARCH_H
#define PLUGINS_AZOTH_INTERFACES_IHAVESEARCH_H
#include <QMetaType>

class QAbstractItemModel;
class QModelIndex;
class QString;

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for search sessions.
	 *
	 * This interface is expected to be implemented by the objects
	 * returned from IHaveSearch::CreateSearchSession() representing
	 * search sessions.
	 * 
	 * The search server string is the via the RestartSearch() method,
	 * which also should automatically restart the search and fetch any
	 * search form, for example.
	 *
	 * The model representing service discovery results is obtained
	 * via GetRepresentationModel() method.
	 *
	 * @sa IHaveServiceDiscovery
	 */
	class ISearchSession
	{
	public:
		virtual ~ISearchSession () {}

		/** @brief Sets the search server used and restarts the search.
		 *
		 * This function should initiate searching on the given server,
		 * if applicable, or "in general", if search server concept is
		 * not applicable to this protocol/account.
		 *
		 * This function should initiate fetching the corresponding
		 * search form and, when it's fetched (asynchronously, of
		 * course), show it to the user, and then, after user's input,
		 * perform the search itself.
		 *
		 * The representation model (see GetRepresentationModel())
		 * should be filled with the search results, if any.
		 *
		 * @param[in] server The search server which should be used for
		 * search.
		 */
		virtual void RestartSearch (QString server) = 0;

		/** @brief Returns the model with the results.
		 *
		 * This function should return the model containing the search
		 * results. The returned model should be the same object for
		 * each call to this function during the lifetime of this
		 * object.
		 *
		 * @return Model with the results.
		 */
		virtual QAbstractItemModel* GetRepresentationModel () const = 0;
	};

	/** @brief Interface for accounts supporting IM search.
	 *
	 * Some protocols may support searching users or other data via IM
	 * services themselves. XEP-0055 (Jabber Search), or typical ICQ oor
	 * Skype users search are the examples of this.
	 *
	 * This interface should be implemented by accounts that support
	 * this feature if they want to provide it for Azoth users in an
	 * uniform way.
	 */
	class IHaveSearch
	{
	public:
		virtual ~IHaveSearch () {}

		/** @brief Creates a new search session.
		 *
		 * This function is called by Azoth core or other plugins
		 * whenever a new search session is required. This function is
		 * expected to return an object implementing ISDSession.
		 * 
		 * The ownership of the returned object is passed to the caller.
		 *
		 * @return An object implementing ISearchSession.
		 *
		 * @sa ISearchSession
		 */
		virtual QObject* CreateSearchSession () = 0;

		/** @brief Returns a default search server for this account.
		 *
		 * This function returns the default search server for this
		 * account. For example, an XMPP implementation would want to
		 * return the same server as the user uses.
		 *
		 * @return The default search server for this account.
		 */
		virtual QString GetDefaultSearchServer () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISearchSession,
		"org.Deviant.LeechCraft.Azoth.ISearchSession/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveSearch,
		"org.Deviant.LeechCraft.Azoth.IHaveSearch/1.0");

#endif
