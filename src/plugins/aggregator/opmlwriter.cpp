/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "opmlwriter.h"
#include <QByteArray>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QtDebug>
#include <util/sll/buildtagstree.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "components/storage/storagebackendmanager.h"

namespace LC
{
namespace Aggregator
{
	OPMLWriter::OPMLWriter (const ITagsManager *itm)
	: TagsManager_ { itm }
	{
	}

	QString OPMLWriter::Write (const channels_shorts_t& channels,
			const QString& title,
			const QString& owner,
			const QString& ownerEmail) const
	{
		QDomDocument doc;
		QDomElement root = doc.createElement ("opml");
		doc.appendChild (root);
		WriteHead (root, doc, title, owner, ownerEmail);
		WriteBody (root, doc, channels);

		return doc.toString ();
	}

	void OPMLWriter::WriteHead (QDomElement& root,
			QDomDocument& doc,
			const QString& title,
			const QString& owner,
			const QString& ownerEmail) const
	{
		QDomElement head = doc.createElement ("head");
		QDomElement text = doc.createElement ("text");
		head.appendChild (text);
		root.appendChild (head);

		if (!title.isEmpty ())
		{
			QDomText t = doc.createTextNode (title);
			text.appendChild (t);
		}
		if (!owner.isEmpty ())
		{
			QDomElement elem = doc.createElement ("owner");
			QDomText t = doc.createTextNode (owner);
			elem.appendChild (t);
			head.appendChild (elem);
		}
		if (!ownerEmail.isEmpty ())
		{
			QDomElement elem = doc.createElement ("ownerEmail");
			QDomText t = doc.createTextNode (ownerEmail);
			elem.appendChild (t);
			head.appendChild (elem);
		}
	}

	void OPMLWriter::WriteBody (QDomElement& root,
			QDomDocument& doc,
			const channels_shorts_t& channels) const
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		auto body = doc.createElement ("body");
		for (const auto& cs : channels)
		{
			auto tags = TagsManager_->GetTags (cs.Tags_);
			tags.sort ();

			auto inserter = Util::BuildTagsTree (tags,
					body, doc, "outline",
					[] (const QDomElement& elem) { return elem.attribute ("text"); },
					[] (QDomElement& result, const QString& tag)
					{
						result.setAttribute ("text", tag);
						result.setAttribute ("isOpen", "true");
					});
			auto item = doc.createElement ("outline");
			item.setAttribute ("title", cs.Title_);
			item.setAttribute ("xmlUrl", sb->GetFeed (cs.FeedID_).URL_);
			item.setAttribute ("htmlUrl", cs.Link_);
			inserter.appendChild (item);
		}

		root.appendChild (body);
	}
}
}
