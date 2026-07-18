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

namespace LC::AN
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
	struct FieldData
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
		QMetaType::Type Type_ = QMetaType::UnknownType;

		/** @brief The types of the event that contain this field.
		 *
		 * This member contains the types of the events that contain
		 * this field. This field won't be checked in events of types
		 * not mentioned here.
		 */
		QStringList EventTypes_;

		struct AllowedValue
		{
			/** @brief Persistent, stable value stored in rules on disk.
			 */
			QByteArray Id_;

			/** @brief Human-readable name of the value.
			 */
			QString Name_;

			bool operator== (const AllowedValue&) const = default;
		};

		using AllowedValues = QList<AllowedValue>;

		/** @brief The allowed values of this field.
		 *
		 * If this list is non-empty, only values from this list are
		 * allowed.
		 *
		 * This only makes sense for Type_ being either
		 * QVariant::String or QVariant::StringList.
		 */
		AllowedValues AllowedValues_ = {};
	};

	/** @brief Describes a field with boolean values.
	 */
	struct BoolFieldValue
	{
		/** @brief Whether the field should be set.
		 */
		bool Value_;

		auto operator<=> (const BoolFieldValue&) const = default;
	};

	/** @brief Describes a field with integer values.
	 */
	struct IntFieldValue
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

		bool operator== (const IntFieldValue&) const = default;
	};

	/** @brief Describes a field with QString values.
	 */
	struct StringFieldValue
	{
		struct Exact
		{
			QString Pattern_;
			bool operator== (const Exact&) const = default;
		};

		struct Substring
		{
			QString Pattern_;
			bool operator== (const Substring&) const = default;
		};

		struct Wildcard
		{
			QString Pattern_;
			QRegularExpression Compiled_ = QRegularExpression::fromWildcard (Pattern_);

			bool operator== (const Wildcard&) const = default;
		};

		struct Pattern : std::variant<Substring, Wildcard, QRegularExpression, Exact>
		{
			using variant::variant;

			explicit(false) Pattern (const QString& str)
			: variant { Exact { str } }
			{
			}

			using Base = variant;
		};

		/** @brief The pattern the values should (not) match.
		 */
		Pattern Matcher_ = Exact {};

		/** @brief Whether the values should match or not match `Matcher_`.
		 *
		 * If this is true, the values should match `Matcher_`, and shouldn't
		 * otherwise.
		 */
		bool Positive_ = true;

		bool operator== (const StringFieldValue&) const = default;
	};

	/** @brief A combination of all possible descriptions.
	 */
	using FieldValue = std::variant<BoolFieldValue, IntFieldValue, StringFieldValue>;
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
 * @sa LC::AN::FieldData
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
	 * Please refer to the documentation of the LC::AN::FieldData
	 * structure for more information.
	 *
	 * @return The list of additional AdvancedNotifications fields.
	 *
	 * @sa LC::AN::FieldData
	 */
	virtual QList<LC::AN::FieldData> GetANFields () const = 0;
};

Q_DECLARE_INTERFACE (IANEmitter, "org.Deviant.LeechCraft.IANEmitter/1.0")
Q_DECLARE_METATYPE (LC::AN::FieldData)
Q_DECLARE_METATYPE (LC::AN::FieldValue)
Q_DECLARE_METATYPE (QList<LC::AN::FieldValue>)

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::AN::IntFieldValue::Operations)
