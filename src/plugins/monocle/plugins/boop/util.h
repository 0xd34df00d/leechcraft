/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <exception>
#include <QString>

class QDomDocument;
class QDomElement;

namespace LC::Monocle::Boop
{
	struct InvalidEpub final : std::exception
	{
		QString Error_;

		InvalidEpub (QString error);

		~InvalidEpub () override = default;

		InvalidEpub (const InvalidEpub&) = delete;
		InvalidEpub (InvalidEpub&&) = delete;
		InvalidEpub& operator= (const InvalidEpub&) = delete;
		InvalidEpub& operator= (InvalidEpub&&) = delete;
	};

	QDomDocument GetXml (const QString& epubFile, const QString& filename);
	QDomElement GetElem (const QDomElement& parent, const QString& tag);
	QString GetAttr (const QDomElement& elem, const QString& name);

	QHash<QString, QDomElement> BuildId2ElementMap (const QDomElement& root);
}
