/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTGEOLOCATION_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTGEOLOCATION_H
#include <QtPlugin>
#include <QMap>
#include <QVariant>

namespace LC
{
namespace Azoth
{
	/** @brief Geolocation data.
	 *
	 * Predefined fields (keys) are based on XEP-0080: User Location
	 * (http://xmpp.org/extensions/xep-0080.html):
	 * - accuracy (double)
	 *   Horizontal GPS error in meters.
	 * - alt (double)
	 *   Altitude in meters above or below sea level.
	 * - area (QString)
	 *   A named area such as a campus or neighborhood.
	 * - bearing (double)
	 *   GPS bearing (direction in which the entity is heading to reach
	 *   its next waypoint), measured in decimal degrees relative to
	 *   true north.
	 * - building (QString)
	 *   A specific building on a street or in an area.
	 * - country (QString)
	 *   The nation where the user is located.
	 * - countrycode (QString)
	 *   The ISO 3166 two-letter country code.
	 * - datum (QString)
	 *   GPS datum.
	 * - description (QString)
	 *   A natural-language name for or description of the location.
	 * - floor (QString)
	 *   A particular floor in a building.
	 * - lat (double)
	 *   Latitude in decimal degrees North.
	 * - locality (QString)
	 *   A locality within the administrative region, such as a town or
	 *   city.
	 * - lon (double)
	 *   Longitude in decimal degrees East.
	 * - postalcode (QString)
	 *   A code used for postal delivery.
	 * - region (QString)
	 *   An administrative region of the nation, such as a state or
	 *   province.
	 * - room (QString)
	 *   A particular room in a building.
	 * - speed (double)
	 *   The speed at which the entity is moving, in meters per second.
	 * - street (QString)
	 *   A thoroughfare within the locality, or a crossing of two
	 *   thoroughfares.
	 * - text (QString)
	 *   A catch-all element that captures any other information about
	 *   the location.
	 * - timestamp (QDateTime)
	 *   UTC timestamp specifying the moment when the reading was taken.
	 * - url (QUrl)
	 *   A URI or URL pointing to information about the location.
	 */
	typedef QMap<QString, QVariant> GeolocationInfo_t;

	/** @brief Interface for accounts supporting geolocation data.
	 *
	 * This interface can be implemented by account objects to advertise
	 * the support for both publishing the current user geolocation data
	 * and fetching other users' data.
	 *
	 * The geolocation concept in Azoth is based on the XMPP XEP-0080:
	 * User Location (http://xmpp.org/extensions/xep-0080.html).
	 *
	 * @sa IAccount
	 */
	class ISupportGeolocation
	{
	public:
		virtual ~ISupportGeolocation () {}

		/** @brief Publishes the given geolocation info.
		 *
		 * If the info map is empty, geolocation info publishing should
		 * be effectively canceled.
		 *
		 * @param[in] info The geolocation info to publish.
		 */
		virtual void SetGeolocationInfo (const GeolocationInfo_t& info) = 0;

		/** @brief Returns info for the given entry and variant.
		 *
		 * If the user identified by an entry doesn't exist, or it has
		 * no variant, or the given combination of user and variant
		 * doesn't publish geolocation information, an empty map should
		 * be returned. The only exception is an empty variant, in this
		 * case this function should return last published info, or info
		 * from the highest priority variant, or whatever is
		 * appropriate.
		 *
		 * @param[in] entry The entry for which to return the info.
		 * @param[in] variant The variant of the entry for which to
		 * return the info.
		 * @return The geolocation information for the given entry and
		 * variant.
		 */
		virtual GeolocationInfo_t GetUserGeolocationInfo (QObject *entry,
				const QString& variant) const = 0;

		/** @brief Notifies about info change of another entry.
		 *
		 * This signal should be emitted whenever geolocation
		 * information is changed for the given variant of the given
		 * contact list entry. The only exception is if the variant
		 * goes offline. In this case, there is no need to emit this
		 * signal.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant The variant for which the event occurred.
		 * @param[out] entry The entry whose geolocation information has
		 * been changed.
		 */
		virtual void geolocationInfoChanged (const QString& variant, QObject *entry) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportGeolocation,
		"org.Deviant.LeechCraft.Azoth.ISupportGeolocation/1.0")

#endif
