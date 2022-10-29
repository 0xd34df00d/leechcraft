/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "atom.h"
#include <QDomDocument>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include "mediarss.h"
#include "utils.h"

namespace LC::Aggregator::Parsers
{
	namespace
	{
		Item_ptr ParseCommonAtomItem (const QDomElement& entry, IDType_t channelId)
		{
			auto item = ParseCommonItem (entry, channelId);

			item->Title_ = Atom::ParseEscapeAware (entry.firstChildElement ("title"));
			item->Link_ = GetLink (entry);
			item->Guid_ = entry.firstChildElement ("id").text ();

			item->Description_ = Atom::ParseEscapeAware (GetBestDescription (entry, { "content", "summary" }));

			return item;
		}

		Item_ptr Parse03Item (const QDomElement& entry, IDType_t channelId)
		{
			auto item = ParseCommonAtomItem (entry, channelId);

			auto date = entry.firstChildElement ("modified");
			if (date.isNull ())
				date = entry.firstChildElement ("issued");
			item->PubDate_ = QDateTime::fromString (date.text (), Qt::ISODateWithMs);

			return item;
		}

		Item_ptr Parse10Item (const QDomElement& entry, IDType_t channelId)
		{
			auto item = ParseCommonAtomItem (entry, channelId);
			item->PubDate_ = QDateTime::fromString (entry.firstChildElement ("updated").text (), Qt::ISODateWithMs);
			return item;
		}

		Channel_ptr ParseAtomChannelCommon (const QDomElement& root, IDType_t feedId)
		{
			auto chan = std::make_shared<Channel> (Channel::CreateForFeed (feedId));
			chan->Title_ = root.firstChildElement ("title").text ().trimmed ();
			if (chan->Title_.isEmpty ())
				chan->Title_ = QObject::tr ("(No title)");
			chan->LastBuild_ = QDateTime::fromString (root.firstChildElement ("updated").text (), Qt::ISODateWithMs);
			chan->Link_ = GetLink (root);
			chan->Author_ = GetAuthor (root);
			return chan;
		}

		bool IsAtom03 (const QDomElement& root)
		{
			if (root.tagName () != "feed" || !root.hasAttribute ("version"))
				return false;
			return root.attribute ("version") == "0.3";
		}

		bool IsAtom10 (const QDomElement& root)
		{
			if (root.tagName () != "feed")
				return false;
			if (!root.hasAttribute ("version"))
				return true;
			return root.attribute ("version", "1.0") == "1.0";
		}
	}

	std::optional<channels_container_t> Atom03 (const QDomDocument& doc, IDType_t feedId)
	{
		const auto& root = doc.documentElement ();
		if (!IsAtom03 (root))
			return {};

		auto chan = ParseAtomChannelCommon (root, feedId);
		chan->Description_ = root.firstChildElement ("tagline").text ();
		chan->Items_ = Util::MapAs<QVector> (Util::DomChildren (root, "entry"),
				[cid = chan->ChannelID_] (const QDomElement& entry) { return Parse03Item (entry, cid); });

		return { { chan } };
	}

	std::optional<channels_container_t> Atom10 (const QDomDocument& doc, IDType_t feedId)
	{
		const auto& root = doc.documentElement ();
		if (!IsAtom10 (root))
			return {};

		auto chan = ParseAtomChannelCommon (root, feedId);
		chan->Description_ = root.firstChildElement ("subtitle").text ();
		if (chan->Author_.isEmpty ())
		{
			const auto& author = root.firstChildElement ("author");
			const auto& name = author.firstChildElement ("name").text ();
			const auto& email = author.firstChildElement ("email").text ();
			if (!name.isEmpty () && !email.isEmpty ())
				chan->Author_ = name + " (" + email + ")";
			else if (!name.isEmpty ())
				chan->Author_ = name;
			else if (!email.isEmpty ())
				chan->Author_ = email;
		}

		chan->Items_ = Util::MapAs<QVector> (Util::DomChildren (root, "entry"),
				[cid = chan->ChannelID_] (const QDomElement& entry) { return Parse10Item (entry, cid); });

		return { { chan } };
	}
}
