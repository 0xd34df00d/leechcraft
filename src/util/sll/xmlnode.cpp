/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlnode.h"
#include <QXmlStreamWriter>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>

namespace LC::Util
{
	Nodes operator+ (Node&& node, Nodes&& nodes)
	{
		nodes.prepend (std::move (node));
		return nodes;
	}

	Nodes operator+ (Nodes&& nodes, Node&& node)
	{
		nodes.push_back (std::move (node));
		return nodes;
	}

	Nodes operator+ (Node&& n1, Node&& n2)
	{
		return { std::move (n1), std::move (n2) };
	}

	Tag Tag::WithText (const QString& name, const QString& contents)
	{
		return { .Name_ = name, .Children_ = { contents } };
	}

	namespace
	{
		void TagToHtml (const Tag& tag, QXmlStreamWriter& w)
		{
			w.writeStartElement (tag.Name_);

			for (const auto& [name, value] : tag.Attrs_)
				w.writeAttribute (name, value);

			for (const auto& node : tag.Children_)
				Util::Visit (node,
						[&w] (const QString& str) { w.writeCharacters (str); },
						[&w] (const Tag& childTag) { TagToHtml (childTag, w); });

			w.writeEndElement ();
		}
	}

	template<HtmlRepr T>
	T Tag::ToHtml (T result) const
	{
		if (Name_.isEmpty ())
			return {};

		QXmlStreamWriter w { &result };
		TagToHtml (*this, w);
		return result;
	}

	template QString Tag::ToHtml (QString) const;
	template QByteArray Tag::ToHtml (QByteArray) const;

	namespace Tags
	{
		UTIL_SLL_API const Tag Br { .Name_ = QStringLiteral ("br") };

		Tag Image (const QString& url)
		{
			return { .Name_ = "img"_qs, .Attrs_ = { { "src"_qs, url } } };
		}

		Tag Li (Nodes&& children)
		{
			return { .Name_ = "li"_qs, .Children_ = std::move (children) };
		}

		Tag Ul (Nodes&& children)
		{
			return { .Name_ = "ul"_qs, .Children_ = std::move (children) };
		}
	}
}
