/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xbelparser.h"
#include <stdexcept>
#include <QObject>
#include <QDomDocument>
#include <util/sll/debugprinters.h>
#include "core.h"

namespace LC
{
namespace Poshuku
{
	XbelParser::XbelParser (const QByteArray& data)
	{
		QDomDocument document;
		if (const auto parseResult = document.setContent (data, QDomDocument::ParseOption::UseNamespaceProcessing);
			!parseResult)
		{
			qWarning () << parseResult;
			throw std::runtime_error (QObject::tr ("XML parse error.").toStdString ());
		}

		QDomElement root = document.documentElement ();
		if (root.tagName () != "xbel")
			throw std::runtime_error (QObject::tr ("Not an XBEL entity.").toStdString ());
		if (root.hasAttribute ("version") &&
				root.attribute ("version") != "1.0")
			throw std::runtime_error (QObject::tr ("This XBEL is not 1.0.").toStdString ());

		QDomElement child = root.firstChildElement ("folder");
		while (!child.isNull ())
		{
			ParseFolder (child);
			child = child.nextSiblingElement ("folder");
		}
	}

	void XbelParser::ParseFolder (const QDomElement& element, QStringList previous)
	{
		QString tag = element.firstChildElement ("title").text ();
		if (!tag.isEmpty () && !previous.contains (tag))
			previous << tag;

		QDomElement child = element.firstChildElement ();
		while (!child.isNull ())
		{
			if (child.tagName () == "folder")
				ParseFolder (child, previous);
			else if (child.tagName () == "bookmark")
				Core::Instance ().GetFavoritesModel ()->
					addItem (child.firstChildElement ("title").text (),
							child.attribute ("href"),
							previous);

			child = child.nextSiblingElement ();
		}
	}
}
}
