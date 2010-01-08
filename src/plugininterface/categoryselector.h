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

#ifndef PLUGININTERFACE_CATEGORYSELECTOR_H
#define PLUGININTERFACE_CATEGORYSELECTOR_H
#include <QTreeWidget>
#include "piconfig.h"

class QStringList;
class QString;

namespace LeechCraft
{
	namespace Util
	{
		/** @brief The CategorySelector widget provides a way to select amongst
		 * a group of items.
		 *
		 * The CategorySelector is a QWidget having Qt::Tool window hint. That
		 * results in representing this widget as a tool window - usually a small
		 * window with smaller than usual title bar and decoration.
		 * CategorySelector represents the possible selections as a list of
		 * check boxes.
		 *
		 * Programmer can set the list of possible choice variants using
		 * SetPossibleSelections and get selected items with GetSelections.
		 *
		 * CategorySelector emits selectionChanged() signal when user changes
		 * his selection. CategorySelector's primary purpose is to help user to
		 * select tags using a line edit, so there's a convenience slot
		 * lineTextChanged() which could be used to notify CategorySelector
		 * about changes of possible categories. There are also convenience
		 * slots selectAll() and selectNone() which could be used to mark all
		 * and no elements in the list respectively.
		 */
		class PLUGININTERFACE_API CategorySelector : public QTreeWidget
		{
			Q_OBJECT
		public:
			/** @brief Constructor.
			 *
			 * Sets the default window title and window flags
			 * (Qt::Tool | Qt::WindowStaysOnTopHint), calculates the
			 * default geometry.
			 * 
			 * @param[in] parent Pointer to parent widget.
			 */
			CategorySelector (QWidget *parent = 0);
			virtual ~CategorySelector ();

			/** @brief Sets possible selections.
			 *
			 * Clears previous selections list, sets new possible selections
			 * according to selections parameter. By default, no items are
			 * selected.
			 *
			 * @param[in] selections Possible selections.
			 *
			 * @sa GetSelections
			 */
			void SetPossibleSelections (const QStringList& selections);
			/** @brief Gets selected items.
			 *
			 * Returns the selected items - a subset of selection variants
			 * passed via SetPossibleSelections.
			 *
			 * @return Selected items.
			 *
			 * @sa SetPossibleSelections
			 */
			QStringList GetSelections ();
		protected:
			/** @brief Checks whether after the move event the selector
			 * won't be beoynd the screen. if it would, moves back.
			 */
			virtual void moveEvent (QMoveEvent*);
		public slots:
			/** @brief Selects all variants.
			 */
			void selectAll ();
			/** @brief Deselects all variants.
			 */
			void selectNone ();
			/** @brief Notifies CategorySelector about logical selection
			 * changes.
			 *
			 * This slot is usually used to notify CategorySelector about
			 * selection changes done via a related widget - for example, a line
			 * edit with tags.
			 *
			 * @param[in] newText The text of the line edit.
			 */
			void lineTextChanged (const QString& newText);
		private slots:
			/** @brief Emits selectionChanged() to notify about selection changes.
			 */
			void buttonToggled ();
		signals:
			/** @brief Indicates that selections have changed.
			 *
			 * @param[out] newSelections Selected items.
			 */
			void selectionChanged (const QStringList& newSelections);
		};
	};
};

#endif

