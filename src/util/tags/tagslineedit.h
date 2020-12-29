/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QLineEdit>
#include "tagsconfig.h"
#include "categoryselector.h"

namespace LC::Util
{
	class TagsCompleter;
	class LineEditButtonManager;

	/** @brief A line edit class suitable for use with TagsCompleter.
	 *
	 * One would need this extra class because of custom behavior of both
	 * tags completer and line edit semantics.
	 *
	 * @sa TagsCompleter
	 */
	class UTIL_TAGS_API TagsLineEdit : public QLineEdit
	{
		Q_OBJECT

		friend class TagsCompleter;

		CategorySelector *CategorySelector_;
		TagsCompleter *Completer_ = nullptr;

		QString Separator_;
	public:
		/** @brief Constructs the line edit widget.
		 *
		 * Creates the line edit widget.
		 *
		 * @param[in] parent Parent widget.
		 */
		explicit TagsLineEdit (QWidget *parent);

		/** @brief Adds the selector widget to the line edit.
		 *
		 * Because this function uses the completion model, it should be
		 * used after a TagsCompleter has been set on this line edit.
		 *
		 * This function also creates an overlay button to aid user in
		 * selecting tags. The passed \em manager object is used (if it
		 * is not nullptr), otherwise a new LineEditButtonManager is
		 * created internally to manage this line edit.
		 *
		 * @param[in] manager The line edit buttons manager to use, or
		 * nullptr to create one.
		 *
		 * @sa TagsCompleter
		 */
		void AddSelector (LineEditButtonManager *manager = nullptr);

		void AddSelector (CategorySelector*);

		/** @brief Returns the separator for the tags.
		 *
		 * @sa SetSeparator()
		 * @sa GetDefaultTagsSeparator ()
		 */
		QString GetSeparator () const;

		/** @brief Sets the separator for the tags.
		 *
		 * This function doesn't update the text in the line edit.
		 *
		 * @sa GetSeparator()
		 */
		void SetSeparator (const QString&);
	public slots:
		/** @brief Completes the string.
		 *
		 * Completes the current text in line edit with completion passed
		 * throught string parameter.
		 *
		 * @param[in] string String with completion.
		 */
		void insertTag (const QString& string);

		/** @brief Sets thew new list of the available tags.
		 *
		 * The list of tags will be passed to the selector if it was
		 * added via AddSelector().
		 *
		 * @param[in] allTags The list of new available tags.
		 */
		void handleTagsUpdated (const QStringList& allTags);

		/** @brief Sets the currently selected tags.
		 *
		 * Sets the line edit text to tags joined by separator. If
		 * tags selector is installed via AddSelector(), the selector
		 * is updated as well.
		 *
		 * @param[in] tags The list of selected tags.
		 */
		void setTags (const QStringList& tags);
	private slots:
		void handleSelectionChanged (const QStringList&);
		void showSelector ();
	protected:
		void keyPressEvent (QKeyEvent*) override;
		void focusInEvent (QFocusEvent*) override;
		void contextMenuEvent (QContextMenuEvent*) override;
		void SetCompleter (TagsCompleter*);
	private:
		QString textUnderCursor () const;
	signals:
		void tagsChosen ();
	};
}
