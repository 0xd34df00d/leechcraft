/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QDialog>
#include "tagsconfig.h"

class QStringList;
class QString;

namespace Ui
{
	class CategorySelector;
}

namespace LC::Util
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
	class UTIL_TAGS_API CategorySelector : public QDialog
	{
		Q_OBJECT

		std::shared_ptr<Ui::CategorySelector> Ui_;

		QString Caption_;
		QString Separator_;
	public:
		enum class ButtonsMode
		{
			NoButtons,
			Close,
			AcceptReject
		};

		/** @brief Constructor.
		 *
		 * Sets the default window title and window flags
		 * (Qt::Tool | Qt::WindowStaysOnTopHint), calculates the
		 * default geometry.
		 *
		 * @param[in] parent Pointer to parent widget.
		 */
		CategorySelector (QWidget *parent = 0);

		/** @brief Sets the caption of this selector.
		 *
		 * By default, the selector has no caption.
		 *
		 * @param[in] caption The new caption of this selector.
		 */
		void SetCaption (const QString& caption);

		/** @brief Gets selected items.
		 *
		 * Returns the selected items - a subset of selection variants
		 * passed via SetPossibleSelections.
		 *
		 * @return Selected items.
		 *
		 * @sa SetPossibleSelections()
		 * @sa GetSelectedIndexes()
		 */
		QStringList GetSelections () const;

		/** @brief Gets the indexes of the selected items.
		 *
		 * Returns the indexes of the selected items in the array
		 * passed to setPossibleSelections(). Please note that
		 * sorting should be disabled in setPossibleSelections()
		 * for this function to be useful.
		 *
		 * @sa GetSelections()
		 */
		QList<int> GetSelectedIndexes () const;

		/** @brief Selects some of the items.
		 *
		 * Selects some of the items presented by elements of the
		 * subset list.
		 *
		 * This function won't emit selectionChanged() signal.
		 *
		 * @param[in] subset The list of items to select.
		 */
		void SetSelections (const QStringList& subset);

		/** @brief Returns the separator for the tags.
		 *
		 * @sa SetSeparator()
		 * @sa GetDefaultTagsSeparator()
		 */
		QString GetSeparator () const;

		/** @brief Sets the separator for the tags.
		 *
		 * This function doesn't update the text in the line edit.
		 *
		 * @sa GetSeparator()
		 */
		void SetSeparator (const QString&);

		/** @brief Sets the buttons mode.
		 */
		void SetButtonsMode (ButtonsMode);
	protected:
		/** @brief Checks whether after the move event the selector
		 * won't be beoynd the screen. if it would, moves back.
		 */
		virtual void moveEvent (QMoveEvent*);

		/** @brief Sets possible selections.
		 *
		 * Clears previous selections list, sets new possible selections
		 * according to selections parameter. By default, no items are
		 * selected.
		 *
		 * The \em selections list is sorted unless the \em sort
		 * parameter is set to false. Please note that if you plan to
		 * call GetSelectedIndexes() you should set \em sort to
		 * false.
		 *
		 * @param[in] selections Possible selections.
		 * @param[in] sort Whether the selections should be sorted
		 * (default is true).
		 *
		 * @sa GetSelections()
		 * @sa GetSelectedIndexes()
		 */
		void SetPossibleSelections (QStringList selections, bool sort = true);

		/** @brief Selects all variants.
		 */
		void SelectAll ();

		/** @brief Deselects all variants.
		 */
		void SelectNone ();
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
		void tagsSelectionChanged (const QStringList& newSelections);
	};
}
