/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlnode.h"
#include <QSize>
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

	Tag& Tag::WithAttr (QString key, QString value) &&
	{
		Attrs_.push_back ({ std::move (key), std::move (value) });
		return *this;
	}

	namespace Tags
	{
		UTIL_SLL_API const Tag Br { .Name_ = QStringLiteral ("br") };

		Tag Html (Nodes&& children)
		{
			return
			{
				.Name_ = "html"_qs,
				.Attrs_ = { { "xmlns"_qs, "http://www.w3.org/1999/xhtml" } },
				.Children_ = std::move (children),
			};
		}

		Tag Charset (const QString& charset)
		{
			return { .Name_ = "meta"_qs, .Attrs_ = { { "charset"_qs, charset } } };
		}

		Tag Title (const QString& title)
		{
			return { .Name_ = "title"_qs, .Children_ = { title } };
		}

		Tag Style (const QString& style)
		{
			return { .Name_ = "style"_qs, .Children_ = { style } };
		}

		Tag Body (Nodes&& children)
		{
			return { .Name_ = "body"_qs, .Children_ = std::move (children) };
		}

		Tag Image (const QString& url)
		{
			return { .Name_ = "img"_qs, .Attrs_ = { { "src"_qs, url } } };
		}

		Tag Image (const QString& url, const QSize& size)
		{
			const auto& w = QString::number (size.width ());
			const auto& h = QString::number (size.height ());
			return
			{
				.Name_ = "img"_qs,
				.Attrs_ = { { "src"_qs, url }, { "width"_qs, w }, { "height"_qs, h } },
			};
		}

		Tag Li (Nodes&& children)
		{
			return { .Name_ = "li"_qs, .Children_ = std::move (children) };
		}

		Tag Ul (Nodes&& children)
		{
			return { .Name_ = "ul"_qs, .Children_ = std::move (children) };
		}

		Tag P (Nodes&& children)
		{
			return { .Name_ = "p"_qs, .Children_ = std::move (children) };
		}

		Nodes TableGrid (size_t rows, size_t cols, const std::function<Nodes (size_t, size_t)>& cell)
		{
			Nodes result;
			result.reserve (rows);

			for (size_t r = 0; r < rows; ++r)
			{
				Nodes rowCells;
				rowCells.reserve (cols);
				for (size_t c = 0; c < cols; ++c)
					rowCells.push_back (Tag { .Name_ = "td"_qs, .Children_ = cell (r, c) });

				result.push_back (Tag { .Name_ = "tr"_qs, .Children_ = std::move (rowCells) });
			}

			return result;
		}
	}
}
