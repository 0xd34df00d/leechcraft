/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QPair>
#include <QList>
#include <QRegExp>

class QDomElement;

/** @brief Interface for HTML/WYSIWYG editors with some advanced functionality.
 *
 * @section CustomTags Custom tags
 *
 * It is sometimes desirable to add support for custom tags not defined
 * in HTML standards, like LJ's user tags. This is done via the
 * SetCustomTags() method, which operates on a list of CustomTag
 * structures.
 *
 * Each custom tag consists of a tag name that is used to identify the
 * tag, and the function that makes HTML out of the given tag with the
 * given attributes and contents. The converter function is invoked each
 * time the web view is should be re-rendered after HTML edit has been
 * modified. See the documentation of CustomTag for more information.
 *
 * All of the above is implemented using XML parsing, so the document
 * should be a valid XML document as well.
 */
class Q_DECL_EXPORT IAdvancedHTMLEditor
{
public:
	virtual ~IAdvancedHTMLEditor () {}

	typedef QPair<QRegExp, QString> Replacement_t;
	typedef QList<Replacement_t> Replacements_t;

	/** @brief Describes a single custom tag.
	 */
	struct CustomTag
	{
		/** @brief The name of the custom tag, like \em lj.
		 */
		QString TagName_;

		/** @brief The converter of an instance of the tag to HTML.
		 *
		 * This function is invoked to convert an instance of the tag
		 * (passed as a QDomElement) to HTML. The conversion should be
		 * done in-place: the resulting HTML should be contained in the
		 * passed QDomElement.
		 *
		 * An example function that boldifies
		 * <code>&lt;lj user="$username"/></code>:
		 *
		 * \code
			[] (QDomElement& elem)
			{
				const auto& user = elem.attribute ("user");
				elem.setTagName ("strong");
				elem.removeAttribute ("user");
				elem.appendChild (elem.ownerDocument ().createTextNode (user));
			}
		   \endcode
		 */
		std::function<void (QDomElement&)> ToKnown_;

		/** @brief The converter of an instance of the tag from HTML.
		 *
		 * This function is invoked to convert an instance of the tag
		 * (passed as a QDomElement) from HTML. The conversion should be
		 * done in-place: the resulting XML should be contained in the
		 * passed QDomElement.
		 *
		 * An example function that turns back the boldified
		 * <code>&lt;lj user="$username"/></code> got from
		 * CustomTag::ToKnown_:
		 *
		 * \code
			[] (QDomElement& elem)
			{
				const auto& user = elem.text ();
				elem.setTagName ("lj");
				elem.setAttribute ("user", user);

				const auto& childNodes = elem.childNodes ();
				while (!childNodes.isEmpty ())
					elem.removeChild (childNodes.at (0));
			}
		   \endcode
		 *
		 * One can leave this function unset, in this case the tag will
		 * marked as non-modifyable.
		 *
		 * This function should return \em true if the convertation
		 * succeeded, otherwise it should return \em false.
		 */
		std::function<bool (QDomElement&)> FromKnown_;
	};
	typedef QList<CustomTag> CustomTags_t;

	/** @brief Inserts the given HTML at the current cursor position.
	 *
	 * This function is somewhat analogous to DOM's
	 * <code>execCommand("insertHTML", ...)</code>.
	 *
	 * @param[in] html The HTML to insert.
	 */
	virtual void InsertHTML (const QString& html) = 0;

	/** @brief Adds support for custom tags not present in HTML standard.
	 *
	 * This function should be called before ITextEditor::SetContents().
	 *
	 * See the IAdvancedHTMLEditor class reference for more information
	 * about tags mappings.
	 *
	 * @param[in] tags The tags mapping.
	 */
	virtual void SetCustomTags (const CustomTags_t& tags) = 0;

	/** @brief Adds a custom action to wrap selected text into given tag.
	 *
	 * For example, to insert an action to wrap selected text into
	 * <code>&lt;span style="font-weight: bold" id="sometext">...&lt;/span></code>
	 * one should call this function like this:
	 * \code
		QVariantMap params;
		params ["style"] = "font-weight: bold";
		params ["id"] = "sometext";
		auto action = editor->AddInlineTagInserter ("span", params);
		action->setText ("Name of your action");
		// further customize the action
	   \endcode
	 *
	 * @param[in] tagName The name of the tag to be inserted.
	 * @param[in] params The parameters of the tag.
	 */
	virtual QAction* AddInlineTagInserter (const QString& tagName, const QVariantMap& params) = 0;

	/** @brief Executes the given \em js in the context of the content.
	 *
	 * @param[in] js The JavaScript code to execute.
	 */
	virtual void ExecJS (const QString& js) = 0;
};

Q_DECLARE_INTERFACE (IAdvancedHTMLEditor, "org.Deviant.LeechCraft.IAdvancedHTMLEditor/1.0")
