/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <memory>
#include <QMap>
#include <QtPlugin>

class QStandardItem;

namespace Media
{
	class IRadioStation;
	typedef std::shared_ptr<IRadioStation> IRadioStation_ptr;

	/** @brief Describes the type of a radio station.
	 *
	 * @sa IRadioStationProvider
	 */
	enum RadioType
	{
		/** @brief No type (item doesn't correspond to a radio station).
		 */
		None,

		/** @brief Radio of artists similar to a given one.
		 *
		 * When selecting a radio of this type the GUI should present the
		 * user with a dialog where he can enter the desired artist's
		 * name which should be passed to the
		 * IRadioStationProvider::GetRadioStation() method.
		 */
		SimilarArtists,

		/** @brief Radio of a global tag like \em metalcore.
		 *
		 * When selecting a radio of this type the GUI should present the
		 * user with a dialog where he can enter the desired tag name
		 * which should be passed to the
		 * IRadioStationProvider::GetRadioStation() method.
		 */
		GlobalTag,

		/** @brief A predefined radio station like an Icecast stream.
		 */
		Predefined
	};

	/** @brief Custom user roles for the items in the model.
	 *
	 * @sa IRadioStationProvider
	 */
	enum RadioItemRole
	{
		/** @brief The type of this radio station.
		 *
		 * The value should be a member of the RadioType enumeration.
		 */
		ItemType = Qt::UserRole + 1,

		/** @brief The internal ID of the radio.
		 */
		RadioID,

		/** @brief Maximum role.
		 */
		MaxRadioRole
	};

	/** @brief Interface for plugins providing radio stations.
	 *
	 * Plugins that provide access to radio stations, either static like
	 * Icecast streams or dynamic like library radio or similar artists
	 * radio in Last.FM should implement this interface.
	 */
	class Q_DECL_EXPORT IRadioStationProvider
	{
	public:
		virtual ~IRadioStationProvider () {}

		/** @brief Returns a radio station for the given item and query.
		 *
		 * The item should be the one of returned from the
		 * GetRadioListItems() method or its child. The query only makes
		 * sense for RadioType::SimilarArtists and RadioType::GlobalTag
		 * radio station types, where it is the source artist name and
		 * tag name correspondingly. Otherwise it can be any string and
		 * shouldn't be taken into account.
		 *
		 * @param[in] item The item identifying the radio station.
		 * @param[in] query The additional user input, if applicable.
		 * @return The radio station object.
		 */
		virtual IRadioStation_ptr GetRadioStation (QStandardItem *item, const QString& query) = 0;

		/** @brief Returns the list of stations provided by this plugin.
		 *
		 * The returned items should have proper text and icon set, as
		 * well as the RadioType in the RadioItemRole::ItemType data
		 * role.
		 *
		 * The returned items will typically have children and could be
		 * filled and refreshed dynamically, so it is suggested to add
		 * them as is to a QStandardItemModel.
		 *
		 * The ownership of the items is \em not passed to the caller,
		 * they still belong to the provider plugin.
		 *
		 * @return The list of root items.
		 */
		virtual QList<QStandardItem*> GetRadioListItems () const = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IRadioStationProvider, "org.LeechCraft.Media.IRadioStationProvider/1.0");
