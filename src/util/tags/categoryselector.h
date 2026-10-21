/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include <QWidget>
#include "tagsconfig.h"

class QTreeView;

namespace LC::Util
{
	/** @brief A checklist over a model of names.
	 *
	 * The names are the `Qt::DisplayRole` of the first column of the model set
	 * via `SetModel()`, listed sorted. The selection is a set of names: the model
	 * decides which of them are shown and reported, so a name selected while
	 * absent from the model is checked as soon as the model lists it.
	 */
	class UTIL_TAGS_API CategorySelector : public QWidget
	{
		Q_OBJECT

		class CheckableModel;
		CheckableModel& Model_;
		QTreeView& View_;
	public:
		explicit CategorySelector (QWidget *parent = nullptr);

		/** @brief Lists the names of `model`, which is not owned.
		 */
		void SetModel (QAbstractItemModel& model);

		/** @brief Sets the header above the list; there is none by default.
		 */
		void SetCaption (const QString& caption);

		/** @brief Returns the selected names present in the model.
		 *
		 * The names are returned in the model rows order.
		 */
		QStringList GetSelections () const;

		/** @brief Returns the model's indexes of the selected names.
		 *
		 * The names are returned in the model rows order.
		 */
		QModelIndexList GetSelectedIndexes () const;

		/** @brief Replaces the selection.
		 *
		 * `selectionEdited()` is not emitted.
		 */
		void SetSelections (const QStringList& names);

		/** @brief Selects exactly the names in the model.
		 *
		 * `selectionEdited()` is not emitted.
		 */
		void SelectAll ();

		/** @brief Clears the selection.
		 *
		 * `selectionEdited()` is not emitted.
		 */
		void SelectNone ();
	signals:
		/** @brief Emitted after the user has changed the selection, with `GetSelections()`.
		 *
		 * This signal is not emitted on programmatic selection changes.
		 */
		void selectionEdited (const QStringList& selected);
	};
}
