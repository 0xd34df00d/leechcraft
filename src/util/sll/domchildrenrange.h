/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/iterator.hpp>
#include <boost/iterator/function_input_iterator.hpp>
#include <boost/range.hpp>
#include <QDomElement>
#include <QString>

namespace LC
{
namespace Util
{
	namespace detail
	{
		class DomSiblingsIterator : public boost::iterator_facade<
				DomSiblingsIterator,
				QDomElement,
				boost::single_pass_traversal_tag,
				const QDomElement&
			>
		{
			QDomElement Elem_;
			const QString TagName_;
		public:
			DomSiblingsIterator () = default;

			DomSiblingsIterator (const QDomElement& firstChild, const QString& tagName)
			: Elem_ { firstChild }
			, TagName_ { tagName }
			{
			}

			void increment ()
			{
				Elem_ = Elem_.nextSiblingElement (TagName_);
			}

			const QDomElement& dereference () const
			{
				return Elem_;
			}

			bool equal (const DomSiblingsIterator& other) const
			{
				return Elem_ == other.Elem_;
			}
		};
	}

	/** @brief Creates a range iterating over direct children named \em tag.
	 *
	 * The returned range is suitable for range-based for loops, in particular. For instance, the
	 * following is correct:
	 * \code{.cpp}
	 	QDomDocument doc;
	 	doc.setContent (R"(
	 				<root>
	 					<child num="1">first text</child>
	 					<child num="2">second text</child>
	 					<child num="3">third text</child>
	 				</root>
	 			)");

	 	for (const auto& elem : MakeDomSiblingsRange (doc.firstChildElement ("root"), "child"))
	 		qDebug () << elem.text () << elem.attribute ("num");
	   \endcode
	 *
	 * Modifying the underlying DOM tree will result in the same effects as modifying it during the
	 * canonical Qt-way of repeatedly calling <code>QDomElement::nextSiblingElement()</code> until
	 * the element becomes null.
	 *
	 * @param parent The parent element whose children should be iterated over.
	 * @param tag The tag name of the child nodes (or an empty string for .
	 * @return The range object representing the collection of the child nodes.
	 */
	inline auto DomChildren (const QDomNode& parent, const QString& tag)
	{
		auto child = parent.firstChildElement (tag);
		return boost::make_iterator_range<detail::DomSiblingsIterator> ({ child, tag }, {});
	}
}
}
