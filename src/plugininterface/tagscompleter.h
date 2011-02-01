/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGININTERFACE_TAGSCOMPLETER_H
#define PLUGININTERFACE_TAGSCOMPLETER_H
#include <QCompleter>
#include "piconfig.h"

namespace LeechCraft
{
	class TagsManager;

	namespace Util
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

			PLUGININTERFACE_API static QAbstractItemModel *CompletionModel_;
			friend class LeechCraft::TagsManager;
		public:
			/** @brief Constructs the completer.
			 *
			 * Sets up for completion and prepares line for work with itself.
			 *
			 * @param[in] line The line edit which would be used for tag
			 * completion.
			 * @param[in] parent Parent object.
			 */
			PLUGININTERFACE_API TagsCompleter (TagsLineEdit *line,
					QObject *parent = 0);

			/** @brief Replaces the model this completer works with.
			 *
			 * By default, the completer uses global tags model, which
			 * is suitable for autocompletion of tags. If you want to
			 * use a custom model with custom contents, use this method
			 * to override the model used for completion.
			 *
			 * @param[in] model The model to use.
			 */
			PLUGININTERFACE_API void OverrideModel (QAbstractItemModel *model);

			/** @brief Path splitter override.
			 *
			 * Handles sequence of tags considering its set structure. Splits
			 * the path by spaces and returns the resulting string list.
			 *
			 * @param[in] path The tags sequence to split.
			 * @return Splitted sequence.
			 */
			PLUGININTERFACE_API virtual QStringList splitPath (const QString& path) const;
		protected:
			static void SetModel (QAbstractItemModel *model)
			{
				CompletionModel_ = model;
			}
		};
	};
};

#endif

