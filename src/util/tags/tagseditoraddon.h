/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <QStringList>
#include "tagsconfig.h"

class ITagsManager;
class QAbstractItemModel;
class QCompleter;
class QLineEdit;

namespace LC::Util
{
	class CategorySelector;
	class LineEditButtonManager;
	struct TagsChange;

	/** @brief Makes a `QLineEdit` edit a list of tags.
	 *
	 * The token under the cursor is completed from the tags model, the bound
	 * selector lists that model and follows the text, and the user's changes are
	 * reported as set differences via `tagsCommitted()`. The object is owned by the
	 * edit it is attached to.
	 */
	class UTIL_TAGS_API TagsEditorAddon : public QObject
	{
		Q_OBJECT

		QLineEdit& Edit_;
		QCompleter& Completer_;
		QString Separator_;
		QString CompletionPrefix_;
		QStringList ShownTags_;
		QPointer<CategorySelector> Selector_;
	public:
		enum class Flag : std::uint8_t
		{
			None = 0,
			NoSelectorButton = 0b1
		};
		Q_DECLARE_FLAGS (Flags, Flag)

		/** @brief Attaches to `edit` with `model` as the tags model; `model` is not owned.
		 *
		 * Unless `flags` has `Flag::NoSelectorButton`, the selector button is
		 * added as via `AddSelectorButton()`.
		 */
		TagsEditorAddon (QLineEdit& edit, QAbstractItemModel& model, Flags flags = Flag::None);

		/** @brief Attaches to `edit` with the model of `tm` as the tags model, as the model overload does.
		 */
		TagsEditorAddon (QLineEdit& edit, ITagsManager& tm, Flags flags = Flag::None);

		/** @brief Returns the separator the tags are joined by, `GetDefaultTagsSeparator()` by default.
		 */
		QString GetSeparator () const;

		/** @brief Sets the separator. The text is not updated.
		 *
		 * @throws std::invalid_argument if the separator is empty.
		 */
		void SetSeparator (const QString&);

		/** @brief Adds a button, to `manager` or to a new one, opening a popup
		 * `CategorySelectorWindow` bound as via `BindSelector()`.
		 *
		 * The popup counts as the bound selector. The constructor does this
		 * unless given `Flag::NoSelectorButton`; call it directly to use a
		 * shared `manager`.
		 *
		 * @throws std::logic_error if a selector is bound already.
		 */
		void AddSelectorButton (LineEditButtonManager *manager = nullptr);

		/** @brief Binds `selector` to the edit.
		 *
		 * The selector lists the tags model and shows the tags in the text selected.
		 * A choice made in it replaces the tags from the model in the text, keeps
		 * the other tokens, and emits `tagPicked()`. At most one selector is
		 * bound, the popup of `AddSelectorButton()` included.
		 *
		 * @throws std::logic_error if a selector is bound already.
		 */
		void BindSelector (CategorySelector& selector);

		/** @brief Shows `tags`: sets the text to them joined by the separator and
		 * syncs the bound selector. The subsequent `tagsCommitted()` signals are
		 * relative to these tags.
		 */
		void SetTags (const QStringList& tags);

		/** @brief Shows `tags` as via `SetTags()` unless the text differs from the shown tags.
		 */
		void UpdateTags (const QStringList& tags);

		/** @brief Returns the tags in the text, split at the separator and trimmed (see `SplitTags()`).
		 */
		QStringList GetTags () const;
	protected:
		bool eventFilter (QObject*, QEvent*) override;
	private:
		struct TokenRange
		{
			qsizetype Begin_;
			qsizetype End_;
		};
		TokenRange TokenUnderCursor () const;
		QString PrefixUnderCursor () const;

		void Complete ();
		void InsertTag (const QString& completion);
		void ApplyChoice (const QStringList& chosen);

		bool IsEdited () const;
		void ReportEdit ();
	signals:
		/** @brief Emitted after the text has been updated with a pick made in the
		 * selector or the completer, also when the pick leaves the tags intact.
		 */
		void tagPicked ();

		/** @brief Emitted when the user has changed the tags, by a pick or by
		 * editing the text and then pressing Enter or moving the focus away.
		 *
		 * The change is relative to the tags last shown via `SetTags()` or
		 * `UpdateTags()`, or reported by the previous emission, whichever happened
		 * last. Edits keeping the set of the tags intact are not reported.
		 */
		void tagsCommitted (const TagsChange& change);
	};
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::Util::TagsEditorAddon::Flags)
