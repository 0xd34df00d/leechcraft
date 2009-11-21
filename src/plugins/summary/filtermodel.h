/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_SUMMARY_FILTERMODEL_H
#define PLUGINS_SUMMARY_FILTERMODEL_H
#include <QSortFilterProxyModel>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			/** Extends QSortFilterProxyModel by allowing to filter items by
			 * their tags. Tags are obtained via the Core::GetTagsForIndex()
			 * function.
			 */
			class FilterModel : public QSortFilterProxyModel
			{
				Q_OBJECT

				bool NormalMode_;
			public:
				/** Constructs the filter model and sets tagless mode.
				 *
				 * @param[in] parent The parent of this model.
				 */
				FilterModel (QObject *parent = 0);

				/** Sets the tags mode. In this mode the string whatever it is
				 * is interpreted as a list of tags to test for.
				 *
				 * @param[in] tags Whether to use tags mode.
				 */
				void SetTagsMode (bool tags);
			protected:
				/** In normal mode this function calls
				 * QSortFilterProxyModel::filterAcceptsRow(). In tags mode, this
				 * function interprets filterRegExp().pattern() as a
				 * human-readable separated list of tags, requests tags for the
				 * source_row of the source_parent. If either of the lists is
				 * empty it returns true. If item's tag contain filter tags it
				 * returns true. Otherwise it returns false.
				 *
				 * @param[in] source_row The row to check.
				 * @param[in] source_parent The parent of this row.
				 * @return Whether the row is accepted or not.
				 */
				virtual bool filterAcceptsRow (int source_row, const QModelIndex& source_parent) const;
			};
		};
	};
};

#endif

