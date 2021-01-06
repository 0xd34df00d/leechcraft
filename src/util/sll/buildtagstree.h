/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QStringList>

class QDomNode;
class QDomElement;
class QDomDocument;

namespace LC::Util
{
	/** @brief Builds a nested sequence of DOM elements representing
	 * a list of tags.
	 *
	 * This function tries to implement projection from tags to a
	 * hierarchical structure in form of XML. It traverses the tags
	 * list and creates child nodes from the document, appending
	 * the hierarchical structure's tree root to the node. It
	 * returns the parent element to which the item should be
	 * appended.
	 *
	 * For empty tags list it just returns node converted to the
	 * QDomElement.
	 *
	 * @param[in] tags List of tags.
	 * @param[in] node The parent-most node to which all other nodes
	 * are appended.
	 * @param[in] document The document containing all these nodes.
	 * @param[in] elementName The name of the XML element that
	 * carries info about the tags.
	 * @param[in] tagSetter Setter function for the tags for the
	 * given element.
	 * @param[in] tagGetter Getter function for the tags for the
	 * given element.
	 * @return Parent element of the item with tags.
	 */
	QDomElement BuildTagsTree (QStringList tags,
			QDomNode& node,
			QDomDocument& document,
			const QString& elementName,
			const std::function<QString (QDomElement)>& tagGetter,
			const std::function<void (QDomElement&, QString)>& tagSetter);
}
