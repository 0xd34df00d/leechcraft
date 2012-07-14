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

#ifndef INTERFACES_IANEMITTER_H
#define INTERFACES_IANEMITTER_H
#include <QtPlugin>
#include <QVariant>
#include <QStringList>

namespace LeechCraft
{
	/** @brief A single additional AdvancedNotifications field.
	 * 
	 * This data structure describes an additional field in the
	 * AdvancedNotifications notification entities. The field ID (the
	 * name of the corresponding key in LeechCraft::Entity::Additional_
	 * map) is stored in the ID_ member.
	 * 
	 * This structure also carries information about field name, type,
	 * description and such.
	 */
	struct ANFieldData
	{
		/** @brief The field ID.
		 * 
		 * The field ID is the value of the corresponding key in the
		 * LeechCraft::Entity::Additional_ map.
		 */
		QString ID_;
		
		/** @brief The name of the field.
		 * 
		 * This member contains the human-readable name of this field.
		 */
		QString Name_;
		
		/** @brief The description of the field.
		 * 
		 * This member contains the human-readable description of this
		 * field.
		 */
		QString Description_;
		
		/** @brief The type of this field.
		 * 
		 * This member contains the type of the value of this field -
		 * the value for the corresponding key (equal to ID_) in the
		 * LeechCraft::Entity::Additional_ map.
		 */
		QVariant::Type Type_;
		
		/** @brief The types of the event that contain this field.
		 * 
		 * This member contains the types of the events that contain
		 * this field. This field won't be checked in events of types
		 * not mentioned here.
		 */
		QStringList EventTypes_;
		
		/** @brief Constructs an empty field info.
		 * 
		 * The corresponding type is invalid, and all other members are
		 * empty.
		 */
		ANFieldData ()
		: Type_ (QVariant::Invalid)
		{
		}
		
		/** @brief Constructs field with the given info variables.
		 * 
		 * @param[in] id The ID of the field (ID_).
		 * @param[in] name The name of the field (Name_).
		 * @param[in] description The description of the field
		 * (Description_).
		 * @param[in] type The type of the field (Type_).
		 * @param[in] events The list of events for this field
		 * (EventTypes_).
		 */
		ANFieldData (const QString& id,
				const QString& name,
				const QString& description,
				QVariant::Type type,
				QStringList events)
		: ID_ (id)
		, Name_ (name)
		, Description_ (description)
		, Type_ (type)
		, EventTypes_ (events)
		{
		}
	};
}

/** @brief Interface for plugins emitting AdvancedNotifications entries.
 * 
 * This interface should be implemented by plugins that support the
 * AdvancedNotifications framework, emit the corresponding entities and
 * provide additional fields in those entities.
 * 
 * The list of additional fields is described by the list of
 * corresponding structures returned from the GetANFields() member.
 * 
 * If a plugin doesn't define any additional fields, it may choose to
 * not implement this interface.
 * 
 * @sa LeechCraft::ANFieldData
 */
class IANEmitter
{
public:
	virtual ~IANEmitter () {}
	
	/** @brief Returns the list of additional fields.
	 * 
	 * This function returns the list of additional fields and their
	 * semantics that may be present in the notifications emitted by
	 * this plugin.
	 * 
	 * This list must not change during single run session.
	 * 
	 * Please refer to the documentation of the LeechCraft::ANFieldData
	 * structure for more information.
	 * 
	 * @return The list of additional AdvancedNotifications fields.
	 * 
	 * @sa LeechCraft::ANFieldData
	 */
	virtual QList<LeechCraft::ANFieldData> GetANFields () const = 0;
};

Q_DECLARE_INTERFACE (IANEmitter, "org.Deviant.LeechCraft.IANEmitter/1.0");
Q_DECLARE_METATYPE (LeechCraft::ANFieldData);

#endif
