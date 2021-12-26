/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QtPlugin>
#include <QVariant>
#include <QStringList>

namespace LC
{
	/** @brief A single additional AdvancedNotifications field.
	 *
	 * This data structure describes an additional field in the
	 * AdvancedNotifications notification entities. The field ID (the
	 * name of the corresponding key in LC::Entity::Additional_
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
		 * LC::Entity::Additional_ map.
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
		 * LC::Entity::Additional_ map.
		 *
		 * For now only QVariant::Int, QVariant::String and
		 * QVariant::StringList are supported.
		 */
		QVariant::Type Type_ = QVariant::Invalid;

		/** @brief The types of the event that contain this field.
		 *
		 * This member contains the types of the events that contain
		 * this field. This field won't be checked in events of types
		 * not mentioned here.
		 */
		QStringList EventTypes_;

		/** @brief The allowed values of this field.
		 *
		 * If this list is non-empty, only values from this list are
		 * allowed.
		 *
		 * This currently only makes sense for QVariant::String and
		 * QVariant::StringList, in which case each QVariant in this list
		 * should be a QString.
		 */
		QVariantList AllowedValues_ = {};
	};

	/** @brief Describes a field with boolean values.
	 */
	struct ANBoolFieldValue
	{
		/** @brief Whether the field should be set.
		 */
		bool IsSet_;

		auto operator<=> (const ANBoolFieldValue&) const = default;
	};

	/** @brief Describes a field with integer values.
	 */
	struct ANIntFieldValue
	{
		/** @brief The boundary of the field.
		 */
		int Boundary_;

		/** @brief Describes the elementary semantics of Boundary_.
		 */
		enum Operation
		{
			/** @brief The value should be greater than Boundary_.
			 */
			OGreater = 0x01,

			/** @brief The value should be less than Boundary_.
			 */
			OLess = 0x02,

			/** @brief The value should be equal to Boundary_.
			 */
			OEqual = 0x04
		};

		Q_DECLARE_FLAGS (Operations, Operation)

		/** @brief Describe the semantics of Boundary_.
		 *
		 * This is the combination of values in Operation enum.
		 */
		Operations Ops_;

		bool operator== (const ANIntFieldValue&) const = default;
	};

	/** @brief Describes a field with QString values.
	 */
	struct ANStringFieldValue
	{
		/** @brief The regular expression the values should (not) match.
		 */
		QRegExp Rx_;

		/** @brief Whether the values should match or not match Rx_.
		 *
		 * If this is true, the values should match Rx_, and shouldn't
		 * otherwise.
		 */
		bool Contains_;

		/** @brief Constructs the field matcher.
		 *
		 * @param[in] rx The regexp to match.
		 * @param[in] contains Whether the string should or should not
		 * match \em rx.
		 */
		ANStringFieldValue (const QRegExp& rx, bool contains)
		: Rx_ { rx }
		, Contains_ { contains }
		{
		}

		/** @brief Constructs the field matcher for the given \em str.
		 *
		 * This constructor constructs a field matcher that matches (or
		 * does not match if \em contains is false) when the string in
		 * question contains the \em str. It is analogous to the previous
		 * constructor if the regular expression object is constructed as
		 * <code>QRegExp { str, Qt::CaseSensitive, QRegExp::FixedString }</code>.
		 *
		 * @param[in] str The string that should be looked for.
		 * @param[in] contains Whether the string should or should not
		 * contain \em str.
		 */
		ANStringFieldValue (const QString& str, bool contains = true)
		: Rx_ { str, Qt::CaseSensitive, QRegExp::FixedString }
		, Contains_ { contains }
		{
		}

		bool operator== (const ANStringFieldValue&) const = default;
	};

	/** @brief A combination of all possible descriptions.
	 */
	using ANFieldValue = std::variant<ANBoolFieldValue, ANIntFieldValue, ANStringFieldValue>;
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
 * @sa LC::ANFieldData
 */
class Q_DECL_EXPORT IANEmitter
{
public:
	virtual ~IANEmitter () = default;

	/** @brief Returns the list of additional fields.
	 *
	 * This function returns the list of additional fields and their
	 * semantics that may be present in the notifications emitted by
	 * this plugin.
	 *
	 * This list must not change during single run session.
	 *
	 * Please refer to the documentation of the LC::ANFieldData
	 * structure for more information.
	 *
	 * @return The list of additional AdvancedNotifications fields.
	 *
	 * @sa LC::ANFieldData
	 */
	virtual QList<LC::ANFieldData> GetANFields () const = 0;
};

Q_DECLARE_INTERFACE (IANEmitter, "org.Deviant.LeechCraft.IANEmitter/1.0")
Q_DECLARE_METATYPE (LC::ANFieldData)
Q_DECLARE_METATYPE (LC::ANFieldValue)
Q_DECLARE_METATYPE (QList<LC::ANFieldValue>)

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::ANIntFieldValue::Operations)
