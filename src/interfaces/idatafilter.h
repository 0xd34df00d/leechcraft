/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QtPlugin>
#include <QString>
#include <QList>
#include <QIcon>
#include <QMetaType>

/** @brief The type to be used as a data filter callback.
 *
 * The functions suitable to be represented as this type can be used as
 * callbacks in the data filters. That is, the result of the data
 * filtering will be passed to them instead of default processing.
 *
 * Data filter callbacks are better documented in the IDataFilter class
 * documentation.
 *
 * @sa IDataFilter
 */
typedef std::function<void (QVariant)> DataFilterCallback_f;

/** @brief Base interface for data filter plugins.
 *
 * Data filter plugins provide some means to manipulate and alter data.
 * Examples of such plugins are image uploaders to various image bins,
 * text finders, etc.
 *
 * A single data filter plugin can support multiple data filter
 * variants, like particular image bins supported by an image uploader
 * or particular OpenSearch engines supported by an OpenSearch handler.
 * The list of possible data filter variants is returned from the
 * GetFilterVariants() function. The list can be dynamic (that is,
 * different in different calls).
 *
 * Plugins implementing this interface are also expected to implement
 * IEntityHandler, considering (and accepting) entities with MIME
 * "x-leechcraft/data-filter-request". Such entities will contain the
 * entity to filter (like, a piece of text or an image) in the
 * LC::Entity::Entity_ field and may contain the "DataFilter"
 * key in the LC::Entity::Additional_ map with the ID of the
 * exact filter variant to use (if user has already selected it).
 *
 * \section Data filter callbacks
 *
 * The result of the data filter, if any, can be passed to the plugin
 * emitting the entity. For this the emitting plugin shall set the
 * callback function as the value of the \em DataFilterCallback
 * LC::Entity::Additional_ map. The function is expected to be an
 * object of type ::DataFilterCallback_f.
 *
 * @note Not all data filter plugins support callbacks. For example, it
 * doesn't make sense for a Google web search data filter to pass the
 * results page contents or URL to the callback. The behavior is expected
 * to be sane enough, though, for the combination of the filtered entity
 * and callback semantics to make sense.
 *
 * @sa LC::Entity, IEntityHandler
 * @sa DataFilterCallback_f
 */
class Q_DECL_EXPORT IDataFilter
{
public:
	/** @brief Describes a single filter variant supported by this data
	 * filter.
	 */
	struct FilterVariant
	{
		/** @brief The ID of this filter variant.
		 *
		 * This ID will be passed in the Additional_ ["DataFilter"]
		 * field in the Entity structure.
		 */
		QByteArray ID_;

		/** @brief The human-readable name of the filter variant.
		 */
		QString Name_;

		/** @brief The description of the filter variant.
		 */
		QString Description_;

		/** @brief The icon representing the filter variant.
		 */
		QIcon Icon_;
	};

	virtual ~IDataFilter () {}

	/** @brief Returns the string describing the data filter.
	 *
	 * This function should return the human-readable string describing
	 * the data filter plugin in general in an imperative form, like
	 * "Upload image to" for an image uploader or "Search in" for a text
	 * searcher.
	 *
	 * @return The human-readable string like "Search in".
	 */
	virtual QString GetFilterVerb () const = 0;

	/** @brief Returns the list of concrete data filter variants.
	 *
	 * The passed \em data could be used to narrow down the list of
	 * filters. For instance, an image uploader may want to check the
	 * size and dimensions of an image and filter out filters not
	 * supporting those specific parameters.
	 *
	 * @param[in] data The specific piece of data for which the variants
	 * should be returned.
	 * @return The list of exact data filter variants.
	 */
	virtual QList<FilterVariant> GetFilterVariants (const QVariant& data) const = 0;
};

Q_DECLARE_METATYPE (DataFilterCallback_f)
Q_DECLARE_INTERFACE (IDataFilter, "org.Deviant.LeechCraft.IDataFilter/1.0")
