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

namespace LC::Aggregator
{
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

	QString Tag::ToHtml () const
	{
		QString result;
		QXmlStreamWriter w { &result };
		TagToHtml (*this, w);
		return result;
	}

	namespace Tags
	{
		const Tag Br { .Name_ = QStringLiteral ("br") };

		Tag Image (const QString& url)
		{
			return { .Name_ = "img"_qs, .Attrs_ = { { "src"_qs, url } } };
		}
	}
}
