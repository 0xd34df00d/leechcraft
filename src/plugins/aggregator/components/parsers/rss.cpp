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
#include <util/sll/qtutil.h>
#include "mediarss.h"
#include "utils.h"

namespace LC::Aggregator::Parsers
{
	namespace
	{
		bool IsRss091 (const QDomElement& root)
		{
			if (root.tagName () != "rss"_ql)
				return false;
			const auto& version = root.attribute ("version"_qs);
			return version == "0.91"_ql || version == "0.92"_ql;
		}

		bool IsRss10 (const QDomElement& root)
		{
			return root.tagName () == "RDF"_ql;
		}

		bool IsRss20 (const QDomElement& root)
		{
			return root.tagName () == "rss"_ql &&
					root.attribute ("version"_qs) == "2.0"_ql;
		}

		Item_ptr ParseCommonRssRdfItem (const QDomElement& entry, IDType_t channelId)
		{
			auto result = ParseCommonItem (entry, channelId);

			result->Title_ = UnescapeHTML (entry.firstChildElement ("title"_qs).text ());
			result->Link_ = entry.firstChildElement ("link"_qs).text ();

			result->Description_ = GetBestDescription (entry, { "description" }).text ();
			if (const auto& duration = GetITunesDuration (entry);
				!duration.isEmpty ())
			{
				if (!result->Description_.isEmpty ())
					result->Description_ += "<br /><br />"_ql;
				result->Description_ += QObject::tr ("Duration: %1")
						.arg (duration);
			}

			result->Guid_ = entry.firstChildElement ("guid"_qs).text ();

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
					result = QDateTime::fromString (str, "yyyy-MM-dd"_qs);
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
			result->PubDate_ = ParseRfc822Lax (entry.firstChildElement ("pubDate"_qs).text ());
			return result;
		}

		Channel_ptr ParseRssChannelMetadata (const QDomElement& channel, IDType_t feedId)
		{
			auto chan = std::make_shared<Channel> (Channel::CreateForFeed (feedId));
			chan->Title_ = channel.firstChildElement ("title"_qs).text ().trimmed ();
			chan->Description_ = channel.firstChildElement ("description"_qs).text ();
			chan->Link_ = GetLink (channel);
			chan->PixmapURL_ = channel.firstChildElement ("image"_qs).firstChildElement ("url"_qs).text ();
			return chan;
		}

		channels_container_t ParseRssDocument (const QDomElement& root, IDType_t feedId)
		{
			channels_container_t channels;
			for (const auto& channel : Util::DomChildren (root, "channel"_qs))
			{
				auto chan = ParseRssChannelMetadata (channel, feedId);
				chan->Language_ = channel.firstChildElement ("language"_qs).text ();
				chan->Author_ = GetAuthor (channel);
				if (chan->Author_.isEmpty ())
					chan->Author_ = channel.firstChildElement ("managingEditor"_qs).text ();
				if (chan->Author_.isEmpty ())
					chan->Author_ = channel.firstChildElement ("webMaster"_qs).text ();
				chan->Items_ = Util::MapAs<QVector> (Util::DomChildren (channel, "item"_qs),
						[cid = chan->ChannelID_] (const QDomElement& item) { return ParseRssItem (item, cid); });

				chan->LastBuild_ = ParseRfc822Lax (channel.firstChildElement ("lastBuildDate"_qs).text ());
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
		const QString RDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#"_qs;
	}

	std::optional<channels_container_t> Rss10 (const QDomDocument& doc, IDType_t feedId)
	{
		const auto& root = doc.documentElement ();
		if (!IsRss10 (root))
			return {};

		channels_container_t channels;

		QHash<QString, Channel_ptr> item2Channel;
		for (const auto& channel : Util::DomChildren (root, "channel"_qs))
		{
			const auto& seqs = channel.firstChildElement ("items"_qs).elementsByTagNameNS (NS::RDF, "Seq"_qs);
			if (seqs.isEmpty ())
				continue;

			auto chan = ParseRssChannelMetadata (channel, feedId);
			chan->LastBuild_ = GetDCDateTime (channel);

			const auto& seqElem = seqs.at (0).toElement ();
			const auto& lis = seqElem.elementsByTagNameNS (NS::RDF, "li"_qs);
			for (int i = 0; i < lis.size (); ++i)
				item2Channel [lis.at (i).toElement ().attribute ("resource"_qs)] = chan;

			channels.push_back (chan);
		}

		for (const auto& itemDescr : Util::DomChildren (root, "item"_qs))
		{
			const auto& about = itemDescr.attributeNS (NS::RDF, "about"_qs);
			const auto chan = item2Channel.value (about);
			if (!chan)
				continue;

			auto item = ParseCommonRssRdfItem (itemDescr, chan->ChannelID_);
			item->PubDate_ = GetDCDateTime (itemDescr);
			chan->Items_.push_back (std::move (item));
		}

		return {};
	}
}
