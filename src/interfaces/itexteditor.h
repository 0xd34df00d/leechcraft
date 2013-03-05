/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QPair>
#include <QList>
#include <QRegExp>

class QWidget;
class QString;
class QAction;
class QColor;

namespace LeechCraft
{
	/** Possible content types a text editor could handle.
	 */
	enum class ContentType
	{
		/** HTML content. The corresponding editor should provide WYSIWYG
		 * capabilities or at least some fancy highlighting for HTML.
		 */
		HTML,

		/** Well, plain text.
		 */
		PlainText
	};

	/** Enumeration for some standard editor actions.
	 */
	enum class EditorAction
	{
		Find,
		Replace
	};
}

/** @brief Interface to be implemented by editor widgets returned from
 * ITextEditor::GetTextEditor().
 *
 * HTML content can be considered as either HTML (taking all the markup
 * into account) or just plain text (without all the markup, just the
 * user-visible characters). The type parameter to GetContents() and
 * SetContents() methods toggles the way HTML content is considered.
 *
 * In general, ContentType::HTML type is used when the caller considers
 * the contents as HTML-enabled, and ContentType::PlainText otherwise.
 *
 * A text editor widget may also implement IAdvancedHTMLEditor if it
 * supports the functionality required by that interface.
 *
 * @sa IAdvancedHTMLEditor
 */
class Q_DECL_EXPORT IEditorWidget
{
public:
	virtual ~IEditorWidget () {}

	/** @brief Returns the editor contents for the given type.
	 *
	 * Returns the contents of this widget according to type.
	 *
	 * For example, for a WYSIWYG text editor widget with HTML contents
	 * @code <h1>header</h1> @endcode this function should return
	 * @code header @endcode for ContentType::PlainText (prerendering
	 * the text in a sense) and @code <h1>header</h1> @endcode for
	 * ContentType::HTML.
	 *
	 * @param[in] type How contents should be interpreted and returned.
	 * @return The editor contents interpreted according to type.
	 *
	 * @sa SetContents()
	 */
	virtual QString GetContents (LeechCraft::ContentType type) const = 0;

	/** @brief Sets contents of this widget interpreted as of the given
	 * type.
	 *
	 * Sets the contents of this widget to contents according to
	 * contents. If type is ContentType::HTML the contents should be
	 * interpreted as rich text, while if type is ContentType::PlainText
	 * the contents should be interpreted as plain text. That is,
	 * @code <h1>header</h1> @endcode should be shown as a big h1 header
	 * in the first case and as is, with tags, in the second.
	 *
	 * Another example is a string like @code <p> @endcode, which should
	 * be kept as is by an HTML editor if the type is ContentType::HTML,
	 * but converted to @code &lt;p> @endcode if the type is
	 * ContentType::PlainText.
	 *
	 * @param[in] contents The new contents of this widget.
	 * @param[in] type The type of the contents.
	 *
	 * @sa GetContents()
	 */
	virtual void SetContents (const QString& contents, LeechCraft::ContentType type) = 0;

	/** @brief Returns a standard editor action.
	 *
	 * Returns the given standard editor action or null if no such action
	 * is available. Ownership is <em>not</em> passed to the caller.
	 *
	 * @param[in] action The standard action to return.
	 * @return The action or null if not available.
	 */
	virtual QAction* GetEditorAction (LeechCraft::EditorAction action) = 0;

	/** @brief Adds a custom action to the editor toolbar, if any.
	 *
	 * This function adds a custom action to the editor toolbar, if the
	 * widget has any. Ownershit is <em>not</em> passed to the editor.
	 * The action can be later removed by RemoveAction().
	 *
	 * @param[in] action The custom action to add.
	 *
	 * @sa RemoveAction()
	 */
	virtual void AppendAction (QAction *action) = 0;

	virtual void AppendSeparator () = 0;

	/** @brief Removes a custom action from the editor.
	 *
	 * This function removes a custom action previously added by
	 * AppendAction().
	 *
	 * @param[in] action An action previously added by AppendAction().
	 *
	 * @sa AppendAction()
	 */
	virtual void RemoveAction (QAction *action) = 0;

	/** @brief Sets the background color of the editor to color.
	 *
	 * @param[in] color The new background color.
	 */
	virtual void SetBackgroundColor (const QColor& color) = 0;
protected:
	/** @brief Notifies about contents changes.
	 *
	 * This signal is emitted each time contents of this editor widget
	 * change.
	 */
	virtual void textChanged () = 0;
};

/** @brief Interface for HTML/WYSIWYG editors with some advanced functionality.
 *
 * @section TagMappings Tag mappings
 *
 * It is sometimes desirable to add support for custom tags not defined
 * in HTML standards, like LJ's user tags. It is possible to define such
 * custom tags by the means of tag mappings, each tag mapping being
 * a pair of QRegExp and QString, which are fed to QString::replace().
 * The QString can refer to the expressions captured by the QRegExp
 * effectively adding support for custom tag parameters (refer to
 * QString::replace() documentation for more information).
 *
 * There are two lists comprising tag mappings, first being a translation
 * from the view to source (that would translate an LJ user tag into
 * appropriate spans, imgs and other tags) and the other being the
 * the reverse translation (that would turn that stuff back into an LJ
 * user tag). These lists are passed to the SetTagsMappings() function,
 * the first list called html2rich and the second called rich2html.
 */
class Q_DECL_EXPORT IAdvancedHTMLEditor
{
public:
	virtual ~IAdvancedHTMLEditor () {}

	typedef QPair<QRegExp, QString> Replacement_t;
	typedef QList<Replacement_t> Replacements_t;

	/** @brief Inserts the given HTML at the current cursor position.
	 *
	 * This function is somewhat analogous to DOM's
	 * @code execCommand("insertHTML", ...) @endcode.
	 *
	 * @param[in] html The HTML to insert.
	 */
	virtual void InsertHTML (const QString& html) = 0;

	/** @brief Sets tags mapping for this editor widget.
	 *
	 * This function should be set before ITextEditor::SetContents().
	 *
	 * See the IAdvancedHTMLEditor class reference for more information
	 * about tags mappings.
	 *
	 * @param[in] rich2html Mappings for view -> source view conversion.
	 * @param[in] html2rich Mappings for source view -> view conversion.
	 */
	virtual void SetTagsMappings (const Replacements_t& rich2html, const Replacements_t& html2rich) = 0;

	/** @brief Executes the given js in the context of the content.
	 *
	 * @param[in] js The JavaScript code to execute.
	 */
	virtual void ExecJS (const QString& js) = 0;
};

/** @brief Interface for plugins implementing a text editor component.
 *
 * If a plugin can provide a text editor widget for other plugins to use
 * it should implement this interface.
 *
 * For example, plugins like a blog client (Blogique) or mail client
 * (Snails) would use such editor widget to allow the user to write posts
 * or mails.
 */
class Q_DECL_EXPORT ITextEditor
{
public:
	virtual ~ITextEditor () {}

	/** @brief Whether this editor plugin supports editing the content type.
	 *
	 * Plain text editors should return true only for the
	 * ContentType::PlainText type, while WYSIWYG-enabled editors should
	 * return true for ContentType::HTML as well.
	 *
	 * @param[in] type The content type to query.
	 * @return Whether the plugin supports editing the given type.
	 */
	virtual bool SupportsEditor (LeechCraft::ContentType type) const = 0;

	/** @brief Creates and returns a new text editor for the given type.
	 *
	 * This function should create a new text editor widget implementing
	 * IEditorWidget for the given content type. If creation fails for
	 * some reason (like unsupported type) this function should return 0.
	 *
	 * It is generally OK to return a WYSIWYG-enabled editor for the
	 * ContentType::PlainText type as long as it supports editing plain
	 * text.
	 *
	 * @param[in] type The content type for which to create the editor.
	 * @return An editor widget implementing IEditorWidget or nullptr in
	 * case of failure.
	 */
	virtual QWidget* GetTextEditor (LeechCraft::ContentType type) = 0;
};

Q_DECLARE_INTERFACE (IEditorWidget, "org.Deviant.LeechCraft.IEditorWidget/1.0");
Q_DECLARE_INTERFACE (IAdvancedHTMLEditor, "org.Deviant.LeechCraft.IAdvancedHTMLEditor/1.0");
Q_DECLARE_INTERFACE (ITextEditor, "org.Deviant.LeechCraft.ITextEditor/1.0");
