/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rss.h"
#include <QDomDocument>
#include <QtDebug>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include "mediarss.h"
#include "utils.h"

namespace LC::Aggregator::Parsers
{
	namespace
	{
		bool IsRss091 (const QDomElement& root)
		{
			if (root.tagName () != "rss")
				return false;
			const auto& version = root.attribute ("version");
			return version == "0.91" || version == "0.92";
		}

		bool IsRss10 (const QDomElement& root)
		{
			return root.tagName () == "RDF";
		}

		bool IsRss20 (const QDomElement& root)
		{
			return root.tagName () == "rss" &&
					root.attribute ("version") == "2.0";
		}

		Item_ptr ParseCommonRssRdfItem (const QDomElement& entry, IDType_t channelId)
		{
			auto result = ParseCommonItem (entry, channelId);

			result->Title_ = UnescapeHTML (entry.firstChildElement ("title").text ());
			result->Link_ = entry.firstChildElement ("link").text ();

			result->Description_ = GetBestDescription (entry, { "description" }).text ();
			if (const auto& duration = GetITunesDuration (entry);
				!duration.isEmpty ())
			{
				if (!result->Description_.isEmpty ())
					result->Description_ += "<br /><br />";
				result->Description_ += QObject::tr ("Duration: %1")
						.arg (duration);
			}

			result->Guid_ = entry.firstChildElement ("guid").text ();

			result->Enclosures_ += RSS::GetEnclosures (entry, result->ItemID_);

			return result;
		}

		namespace
		{
			QDateTime ParseRfc822Lax (const QString& str)
			{
				if (str.isEmpty ())
					return {};

				auto result = QDateTime::fromString (str, Qt::RFC2822Date);
				if (!result.isValid ())
					result = QDateTime::fromString (str, "yyyy-MM-dd");
				if (!result.isValid ())
				{
					qWarning () << "Can't parse RSS item pubDate: "
							<< str;
					result = QDateTime::currentDateTime ();
				}
				return result;
			}
		}

		Item_ptr ParseRssItem (const QDomElement& entry, IDType_t channelId)
		{
			auto result = ParseCommonRssRdfItem (entry, channelId);
			result->PubDate_ = ParseRfc822Lax (entry.firstChildElement ("pubDate").text ());
			return result;
		}

		Channel_ptr ParseRssChannelMetadata (const QDomElement& channel, IDType_t feedId)
		{
			auto chan = std::make_shared<Channel> (Channel::CreateForFeed (feedId));
			chan->Title_ = channel.firstChildElement ("title").text ().trimmed ();
			chan->Description_ = channel.firstChildElement ("description").text ();
			chan->Link_ = GetLink (channel);
			chan->PixmapURL_ = channel.firstChildElement ("image").firstChildElement ("url").text ();
			return chan;
		}

		channels_container_t ParseRssDocument (const QDomElement& root, IDType_t feedId)
		{
			channels_container_t channels;
			for (const auto& channel : Util::DomChildren (root, "channel"))
			{
				auto chan = ParseRssChannelMetadata (channel, feedId);
				chan->Language_ = channel.firstChildElement ("language").text ();
				chan->Author_ = GetAuthor (channel);
				if (chan->Author_.isEmpty ())
					chan->Author_ = channel.firstChildElement ("managingEditor").text ();
				if (chan->Author_.isEmpty ())
					chan->Author_ = channel.firstChildElement ("webMaster").text ();
				chan->Items_ = Util::MapAs<QVector> (Util::DomChildren (channel, "item"),
						[cid = chan->ChannelID_] (const QDomElement& item) { return ParseRssItem (item, cid); });

				chan->LastBuild_ = ParseRfc822Lax (channel.firstChildElement ("lastBuildDate").text ());
				if (!chan->LastBuild_.isValid ())
					chan->LastBuild_ = chan->Items_.isEmpty () ?
							QDateTime::currentDateTime () :
							chan->Items_.front ()->PubDate_;

				channels.push_back (chan);
			}
			return channels;
		}
	}

	std::optional<channels_container_t> Rss091 (const QDomDocument& doc, IDType_t feedId)
	{
		const auto& root = doc.documentElement ();
		if (!IsRss091 (root))
			return {};

		return ParseRssDocument (root, feedId);
	}

	std::optional<channels_container_t> Rss20 (const QDomDocument& doc, IDType_t feedId)
	{
		const auto& root = doc.documentElement ();
		if (!IsRss20 (root))
			return {};

		return ParseRssDocument (root, feedId);
	}

	namespace NS
	{
		const QString RDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
	}

	std::optional<channels_container_t> Rss10 (const QDomDocument& doc, IDType_t feedId)
	{
		const auto& root = doc.documentElement ();
		if (!IsRss10 (root))
			return {};

		channels_container_t channels;

		QHash<QString, Channel_ptr> item2Channel;
		for (const auto& channel : Util::DomChildren (root, "channel"))
		{
			const auto& seqs = channel.firstChildElement ("items").elementsByTagNameNS (NS::RDF, "Seq");
			if (seqs.isEmpty ())
				continue;

			auto chan = ParseRssChannelMetadata (channel, feedId);
			chan->LastBuild_ = GetDCDateTime (channel);

			const auto& seqElem = seqs.at (0).toElement ();
			const auto& lis = seqElem.elementsByTagNameNS (NS::RDF, "li");
			for (int i = 0; i < lis.size (); ++i)
				item2Channel [lis.at (i).toElement ().attribute ("resource")] = chan;

			channels.push_back (chan);
		}

		for (const auto& itemDescr : Util::DomChildren (root, "item"))
		{
			const auto& about = itemDescr.attributeNS (NS::RDF, "about");
			auto& chan = item2Channel.value (about);
			if (!chan)
				continue;

			auto item = ParseCommonRssRdfItem (itemDescr, chan->ChannelID_);
			item->PubDate_ = GetDCDateTime (itemDescr);
			chan->Items_.push_back (std::move (item));
		}

		return {};
	}
}
