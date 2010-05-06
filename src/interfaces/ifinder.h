/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef INTERFACES_IFINDER_H
#define INTERFACES_IFINDER_H
#include <boost/shared_ptr.hpp>
#include <QStringList>
#include <QHash>
#include <QVariant>

class QAbstractItemModel;

namespace LeechCraft
{
	/** @brief Describes the elementary subrequest.
	 *
	 * This structures holds information about elementary search
	 * subrequest that contains no logical (at least, from LeechCraft's
	 * point of view) operators.
	 */
	struct Request
	{
		/** Search type.
		 */
		enum Type
		{
			RTFixed,		//< Fixed string
			RTWildcard,		//< Wildcard
			RTRegexp,		//< Regular expression
			RTTag			//< Tag filtering
		};

		/** Whether the search should be case sensitive.
		 */
		bool CaseSensitive_;

		/** Search type.
		 */
		Type Type_;

		/** Requested plugin for the search.
		 */
		QString Plugin_;

		/** Requested category for the search.
		 */
		QString Category_;

		/** Search string entered by user.
		 */
		QString String_;
		/** Any additional parameters not recognized by LeechCraft.
		 */
		QHash<QString, QVariant> Params_;
	};
};

/** @brief Represents search results for a single Request.
 *
 * Contains a QAbstractItemModel with the search results representation
 * that would be embedded into the LeechCraft main area. Every single
 * subrequest should represent its results via this model.
 */
class IFindProxy
{
public:
	virtual ~IFindProxy () {}

	/** Returns the model with search results representation.
	 *
	 * @return The model with results.
	 */
	virtual QAbstractItemModel* GetModel () = 0;

	/** @brief Represents the unique ID of this finder type.
	 *
	 * Unique ID may be (and would be, in most cases) dependent upon
	 * the request this IFindProxy represents. This is required in order
	 * to filter out different IFindProxy objects that represent the
	 * same result set.
	 *
	 * Consider your plugin performs the same search for two different
	 * categories. For example, one is site's name, where it searches,
	 * and the other one is "music", because your plugin searches for
	 * music. Then, if user selects both music and the site name, only
	 * one search find proxy should be returned. Thus, this function
	 * allows to avoid such situations.
	 *
	 * @return The string with unique ID.
	 */
	virtual QByteArray GetUniqueSearchID () const = 0;

	/** @brief Returns the list of categories this proxy would return.
	 *
	 * This function should return the list of human-readable
	 * representation of categories that this find proxy's results would
	 * belong into.
	 *
	 * @return The list of categories this proxy would search in.
	 */
	virtual QStringList GetCategories () const = 0;
};

typedef boost::shared_ptr<IFindProxy> IFindProxy_ptr;

/** @brief Base class for search providers.
 *
 * Plugin is handled for the supported categories by GetCategories()
 * when needed, and GetProxy() is called for individual sub-requests.
 * 
 * @sa IFindProxy
 */
class IFinder
{
public:
	virtual ~IFinder () {}

	/** @brief Returns the plugin's categories.
	 *
	 * Returns the categories of requests that this plugin is able to
	 * handle. This list isn't cached, so it can be freely changed in
	 * runtime, though changes to the list won't be visible until user
	 * issues a new search request.
	 *
	 * @return The list of up-to-date categories relevant to this
	 * plugin.
	 */
	virtual QStringList GetCategories () const = 0;

	/** @brief Returns find proxies for the given request.
	 *
	 * Returns a shared pointer to IFindProxy objects that handle this
	 * sub-request r.
	 *
	 * @param[in] r The request that should be handled by the returned
	 * IFindProxy objects.
	 * @return QList of shared pointer to the IFindProxy handling the
	 * request.
	 *
	 * @sa IFindProxy
	 */
	virtual QList<IFindProxy_ptr> GetProxy (const LeechCraft::Request& r) = 0;
};

Q_DECLARE_INTERFACE (IFinder, "org.Deviant.LeechCraft.IFinder/1.0");
Q_DECLARE_INTERFACE (IFindProxy, "org.Deviant.LeechCraft.IFinder/1.0");

#endif

