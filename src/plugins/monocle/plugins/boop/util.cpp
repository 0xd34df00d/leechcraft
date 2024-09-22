/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QDomDocument>
#include <QUrl>
#include <quazip/quazipfile.h>
#include <util/sll/qtutil.h>
#include <util/sll/domchildrenrange.h>

namespace LC::Monocle::Boop
{
	InvalidEpub::InvalidEpub (QString error)
	: Error_ { std::move (error) }
	{
	}

	namespace
	{
		auto ParseXml (auto&& xmlable, const auto& context)
		{
			QDomDocument doc;
			QString errorMsg;
			if (!doc.setContent (xmlable, false, &errorMsg))
				throw InvalidEpub { "unable to parse xml " + context };
			return doc;
		}
	}

	QDomDocument GetXml (const QString& epubFile, const QString& filename)
	{
		QuaZipFile file { epubFile, filename, QuaZip::csInsensitive };
		if (!file.open (QIODevice::ReadOnly))
			throw InvalidEpub { "unable to open " + filename + ": " + file.errorString () };

		return ParseXml (&file, filename);
	}

	QDomElement GetElem (const QDomElement& parent, const QString& tag)
	{
		const auto& result = parent.firstChildElement (tag);
		if (result.isNull ())
			throw InvalidEpub { tag + " is empty" };
		return result;
	}

	QString GetAttr (const QDomElement& elem, const QString& name)
	{
		const auto& attrValue = elem.attribute (name);
		if (attrValue.isEmpty ())
			throw InvalidEpub { name + " is empty" };
		return attrValue;
	}

	namespace
	{
		void BuildId2ElementMap (const QDomElement& elem, QHash<QString, QDomElement>& result)
		{
			if (const auto& id = elem.attribute ("id"_qs); !id.isEmpty ())
				result [QUrl::fromPercentEncoding (id.toUtf8 ())] = elem;

			for (const auto& child : Util::DomChildren (elem, {}))
				BuildId2ElementMap (child, result);
		}
	}

	QHash<QString, QDomElement> BuildId2ElementMap (const QDomElement& root)
	{
		QHash<QString, QDomElement> result;
		// TODO C++23 deducing this
		BuildId2ElementMap (root, result);
		return result;
	}
}
