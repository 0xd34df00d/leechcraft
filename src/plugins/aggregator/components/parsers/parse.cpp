/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "parse.h"
#include <QDomDocument>
#include <QUrl>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "atom.h"
#include "rss.h"
#include "utils.h"

namespace LC::Aggregator::Parsers
{
	namespace
	{
		QString FixItemTitle (QString&& title)
		{
			return std::move (title).trimmed ().simplified ();
		}

		QUrl GetDocumentBase (const QDomElement& root, const QUrl& feedUrl)
		{
			const auto& xmlBase = root.attributeNS ("http://www.w3.org/XML/1998/namespace"_qs, "base"_qs);
			return xmlBase.isEmpty () ? feedUrl : feedUrl.resolved (QUrl { xmlBase });
		}

		void ResolveUrl (QString& url, const QUrl& base)
		{
			if (url.isEmpty ())
				return;

			const QUrl parsed { url };
			if (parsed.isValid () && parsed.isRelative ())
				url = base.resolved (parsed).toString ();
		}

		void ResolveUrls (Item& item, const QUrl& base)
		{
			ResolveUrl (item.Link_, base);
			ResolveUrl (item.CommentsLink_, base);
			ResolveUrl (item.CommentsPageLink_, base);
			for (auto& enclosure : item.Enclosures_)
				ResolveUrl (enclosure.URL_, base);
			for (auto& entry : item.MRSSEntries_)
			{
				ResolveUrl (entry.URL_, base);
				ResolveUrl (entry.CopyrightURL_, base);
				for (auto& thumbnail : entry.Thumbnails_)
					ResolveUrl (thumbnail.URL_, base);
				for (auto& peerLink : entry.PeerLinks_)
					ResolveUrl (peerLink.Link_, base);
			}
		}

		void PostprocessParsed (channels_container_t& channels, const QUrl& base)
		{
			for (const auto& newChannel : channels)
			{
				ResolveUrl (newChannel->Link_, base);
				ResolveUrl (newChannel->PixmapURL_, base);
				if (newChannel->Link_.isEmpty ())
				{
					qWarning () << "detected empty link for"
						<< newChannel->Title_;
					newChannel->Link_ = "about:blank"_qs;
				}
				for (const auto& item : newChannel->Items_)
				{
					item->Title_ = FixItemTitle (std::move (item->Title_));
					ResolveUrls (*item, base);
				}
			}
		}
	}

	std::optional<channels_container_t> TryParse (const QDomDocument& doc, IDType_t feedId, const QUrl& feedUrl)
	{
		static const std::array parsers
		{
			&Atom10,
			&Rss20,
			&Atom03,
			&Rss10,
		};

		const auto& base = GetDocumentBase (doc.documentElement (), feedUrl);
		for (auto parser : parsers)
			if (auto res = parser (doc, feedId))
			{
				PostprocessParsed (*res, base);
				return res;
			}

		return {};
	}
}
