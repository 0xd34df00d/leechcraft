/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIcon>
#include <QString>
#include <QVariant>

namespace LC::DataSources
{
	struct EnumValueInfo
	{
		/** @brief The icon associated with this enum value.
		 */
		QIcon Icon_;

		/** @brief The name of this enum value.
		 */
		QString Name_;

		/** @brief Arbitrary data.
		 */
		QVariant UserData_;
	};

	/** @brief The additional item roles for the XSD data source.
	 *
	 * These roles are used by the `"dataview"` XSD item handler to understand
	 * the data that the associated model contains.
	 *
	 * Each horizontal header item in the model will have the values at these
	 * roles describing the corresponding column.
	 */
	enum DataSourceRole
	{
		/** @brief The type of this field.
		 *
		 * The value should be an element of the DataFieldType enum.
		 *
		 * @sa DataFieldType
		 */
		FieldType = Qt::UserRole + 1,

		/** @brief The values admissible for this field.
		 *
		 * The value should be `QList<EnumValueInfo>`.
		 *
		 * This is only used if FieldType is DataFieldType::Enum.
		 *
		 * @sa EnumValueInfo
		 */
		FieldValues,

		/** @brief Whether the field can(not) be modified after it is created.
		 */
		FieldNonModifiable,
	};

	/** @brief The type of the
	 */
	enum DataFieldType
	{
		/** @brief This field does not support nor need an editor.
		 */
		None,

		/** @brief An arbitrary string as a QString.
		 */
		String,

		/** @brief An URL as a QString.
		 */
		Url,

		/** @brief A path to a local file as a QString.
		 */
		LocalPath,

		/** @brief An integer as an `int`.
		 */
		Integer,

		/** @brief An enumeration, with values from the DataSourceRole::FieldValues list.
		 *
		 * @sa DataSourceRole::FieldValues
		 * @sa EnumValueInfo
		 */
		Enum,

		/** @brief A color as a QColor.
		 */
		Color,

		/** @brief A font as a QFont.
		 */
		Font,
	};
}

Q_DECLARE_METATYPE (LC::DataSources::EnumValueInfo)
