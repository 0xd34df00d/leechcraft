/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCompleter>
#include "tagsconfig.h"

namespace LC
{
	class TagsManager;
}

namespace LC::Util
{
	class TagsLineEdit;

	/** @brief Completer suitable for tag completion.
	 *
	 * Handles tag completions. One would need special class for this
	 * because standard QCompleter doesn't work: tag sequence isn't
	 * hierarchical, it is rather a set.
	 *
	 * Despite the TagsCompleter is designed for tags completion
	 * and, consequently, uses the globally available tags model
	 * from LeechCraft's tags manager, it's possible to use the
	 * completer for other ';'-separated lists of values, by setting
	 * the appropriate completion model via OverrideModel() method.
	 *
	 * @sa TagsCompletionModel
	 * @sa TagsLineEdit
	 */
	class TagsCompleter : public QCompleter
	{
		Q_OBJECT

		UTIL_TAGS_API static QAbstractItemModel *CompletionModel_;
		friend class LC::TagsManager;

		TagsLineEdit *Edit_;
	public:
		/** @brief Constructs the completer.
		 *
		 * Sets up for completion and prepares line for work with itself.
		 *
		 * @param[in] line The line edit which would be used for tag
		 * completion.
		 */
		UTIL_TAGS_API explicit TagsCompleter (TagsLineEdit *line);

		/** @brief Replaces the model this completer works with.
		 *
		 * By default, the completer uses global tags model, which
		 * is suitable for autocompletion of tags. If you want to
		 * use a custom model with custom contents, use this method
		 * to override the model used for completion.
		 *
		 * @param[in] model The model to use.
		 */
		UTIL_TAGS_API void OverrideModel (QAbstractItemModel *model);

		/** @brief Path splitter override.
		 *
		 * Handles sequence of tags considering its set structure. Splits
		 * the path by spaces and returns the resulting string list.
		 *
		 * @param[in] path The tags sequence to split.
		 * @return Splitted sequence.
		 */
		UTIL_TAGS_API QStringList splitPath (const QString& path) const override;
	protected:
		static void SetModel (QAbstractItemModel *model)
		{
			CompletionModel_ = model;
		}
	};
}
