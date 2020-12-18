/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>
#include "tagsconfig.h"

namespace LC
{
namespace Util
{
	/** @brief Provides filter model with additional tags filter mode.
	 *
	 * This model behaves just like a normal QSortFilterProxyModel except
	 * it supports filtering by tags. In this mode row is accepted either
	 * if all tags from the request are found in the row (the default
	 * mode) or if the intersection of the two sets is just non-empty.
	 * The mode can be set via SetTagsInclusionMode() function. The tags
	 * filtering mode itself is enabled via setTagsMode() slot.
	 *
	 * The tags are obtained by splitting the filter pattern by the
	 * separator, which is <em>;</em> by default but can be set via the
	 * SetSeparator() method.
	 */
	class UTIL_TAGS_API TagsFilterModel : public QSortFilterProxyModel
	{
		bool NormalMode_;
		QString Separator_;
	public:
		/** @brief Describes the modes of matching two sets of tags.
		 *
		 * @sa SetTagsInclusionMode()
		 */
		enum class TagsInclusionMode
		{
			/** @brief Tags intersection should be non-empty.
			 *
			 * In other words, at least one tag from the filter string
			 * should be found in the list of tags of a row.
			 */
			Any,

			/** @brief Filter string tags should be a subset of row tags.
			 *
			 * In other words, all of the tags from the filter string
			 * should be found in the list of tags of a row.
			 */
			All
		};
	private:
		TagsInclusionMode TagsMode_;
	public:
		/** @brief Creates the model with the given parent.
		 *
		 * @param[in] parent The parent object of this model.
		 */
		TagsFilterModel (QObject *parent = 0);

		/** @brief Sets the separator for the tags.
		 *
		 * The separator is used to split the regexp filter string (the
		 * one set via <code>setFilterFixedString()</code> method) into
		 * the list of tags.
		 *
		 * Setting this property will invalidate the model if the
		 * <code>dynamicSortFilter()</code> property is true.
		 *
		 * @param[in] separator The separator string.
		 *
		 * @sa GetDefaultTagsSeparator()
		 */
		void SetSeparator (const QString& separator);

		/** @brief Sets the tags inclusion mode.
		 *
		 * Setting this property will invalidate the model if the
		 * <code>dynamicSortFilter()</code> property is true.
		 *
		 * @param[in] mode The tags inclusion mode.
		 */
		void SetTagsInclusionMode (TagsInclusionMode mode);

		/** @brief Sets whether the tags filtering mode is enabled.
		 *
		 * By default the tags mode is disabled.
		 *
		 * @param[in] enabled Whether the tags mode should be enabled.
		 */
		void SetTagsMode (bool enabled);
	protected:
		/** @brief Reimplemented from QSortFilterProxyModel::filterAcceptsRow().
		 */
		virtual bool filterAcceptsRow (int, const QModelIndex&) const;

		/** @brief Returns the list of tags for the given row.
		 *
		 * This function should return the list of tags for the given
		 * row. Reimplement it in your subclass to provide this filter
		 * model with correct tags for rows.
		 *
		 * @param[in] row The source row for which tags should be fetched.
		 * @return The list of tags for the \em row.
		 */
		virtual QStringList GetTagsForIndex (int row) const = 0;
	private:
		bool FilterNormalMode (int, const QModelIndex&) const;
		bool FilterTagsMode (int, const QModelIndex&) const;
	};
};
}
