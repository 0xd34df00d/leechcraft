/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLineEdit>
#include "tagsconfig.h"
#include "categoryselector.h"

namespace LC::Util
{
	class TagsCompleter;
	class LineEditButtonManager;
	struct TagsChange;

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

		CategorySelector *CategorySelector_ = nullptr;
		TagsCompleter *Completer_ = nullptr;

		QString Separator_;
		QStringList ShownTags_;
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
		 * The selector offers the tags from the completion model and
		 * follows its changes.
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

		/** @brief Shows the given tags.
		 *
		 * Sets the line edit text to the tags joined by the separator. If
		 * a tags selector is installed via AddSelector(), the selector is
		 * updated as well.
		 *
		 * The subsequent tagsEdited() signals describe the user's changes
		 * relative to these tags.
		 *
		 * @param[in] tags The tags to show.
		 *
		 * @sa UpdateTags()
		 * @sa tagsEdited()
		 */
		void SetTags (const QStringList& tags);

		/** @brief Shows the given tags unless the user is editing the text.
		 *
		 * This function is for showing the changes of the tags shown via
		 * SetTags() that happen elsewhere: the given tags are shown as if
		 * via SetTags() unless the text currently differs from the shown
		 * tags.
		 *
		 * @param[in] tags The tags to show.
		 *
		 * @sa SetTags()
		 */
		void UpdateTags (const QStringList& tags);

		/** @brief Returns the tags in the line edit text.
		 *
		 * @sa SplitTags()
		 */
		QStringList GetTags () const;
	protected:
		void keyPressEvent (QKeyEvent*) override;
		void focusInEvent (QFocusEvent*) override;
		void contextMenuEvent (QContextMenuEvent*) override;
		void SetCompleter (TagsCompleter*);
	private:
		void InsertTag (const QString& completion);

		struct TokenRange
		{
			qsizetype Begin_;
			qsizetype End_;
		};
		TokenRange TokenUnderCursor () const;
		QString TextUnderCursor () const;

		bool IsEdited () const;
		void ReportEdit ();
	signals:
		/** @brief Emitted after the text has been updated with the tags the
		 * user has chosen in the selector or the completer.
		 *
		 * Editing the text by hand does not emit this signal, while a
		 * choice leaving the tags intact does.
		 *
		 * @sa tagsEdited()
		 */
		void tagsChosen ();

		/** @brief Emitted when the user has changed the tags.
		 *
		 * The user changes the tags by choosing them in the selector or
		 * the completer, or by editing the text by hand and then pressing
		 * Enter or moving the focus away.
		 *
		 * The change is relative to the tags last shown via SetTags() or
		 * UpdateTags(), or reported by the previous emission of this
		 * signal, whichever happened last. Edits keeping the set of the
		 * tags intact are not reported.
		 *
		 * @param[out] change The change of the tags.
		 */
		void tagsEdited (const TagsChange& change);
	};
}
