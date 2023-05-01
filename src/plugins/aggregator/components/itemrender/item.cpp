/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "item.h"
#include <QGuiApplication>
#include <QFileInfo>
#include <QPalette>
#include <QUrl>
#include <util/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include "mrss.h"
#include "utils.h"
#include "../../item.h"
#include "../../xmlsettingsmanager.h"

namespace LC::Aggregator
{
	namespace
	{
		QString GetHex (QPalette::ColorRole role, QPalette::ColorGroup group = QPalette {}.currentColorGroup ())
		{
			return QPalette {}.color (group, role).name (QColor::HexRgb);
		}

		Nodes MakeItemLink (const Item& item)
		{
			return { MakeLink (item.Link_, Tag::WithText ("strong", item.Title_)), Tags::Br };
		}

		Nodes MakePublishedInfo (const Item& item)
		{
			if (!item.PubDate_.isValid () && item.Author_.isEmpty ())
				return {};

			QString info;
			if (item.PubDate_.isValid () && !item.Author_.isEmpty ())
				info = TrCtx::tr ("Published on %1 by %2")
						.arg (item.PubDate_.toString (), item.Author_);
			else if (item.PubDate_.isValid ())
				info = TrCtx::tr ("Published on %1")
						.arg (item.PubDate_.toString ());
			else if (!item.Author_.isEmpty ())
				info = TrCtx::tr ("Published by %1")
						.arg (item.Author_);

			return { std::move (info), Tags::Br };
		}

		Nodes MakeCategories (const Item& item)
		{
			if (item.Categories_.isEmpty ())
				return {};

			return { item.Categories_.join ("; "_qs), Tags::Br };
		}

		Nodes MakeComments (const Item& item)
		{
			if (item.NumComments_ < 0 && item.CommentsPageLink_.isEmpty ())
				return {};

			auto userText = item.NumComments_ >= 0 ?
					TrCtx::tr ("%n comment(s)", nullptr, item.NumComments_) :
					TrCtx::tr ("View comments");
			auto textNode = item.CommentsPageLink_.isEmpty () ?
					Node { std::move (userText) } :
					MakeLink (item.CommentsPageLink_, userText);
			return { std::move (textNode), Tags::Br };
		}

		Nodes MakeGeolocation (const Item& item)
		{
			if (!static_cast<bool> (item.Latitude_) && !static_cast<bool> (item.Longitude_))
				return {};

			const auto& latStr = QString::number (item.Latitude_);
			const auto& lonStr = QString::number (item.Longitude_);

			const auto& link = "https://maps.google.com/maps?f=q&source=s_q&hl=en&geocode=&q=%1+%2"_qs
					.arg (latStr, lonStr);
			const auto& text = TrCtx::tr ("Geoposition:") + ' ' + latStr + ' ' + lonStr;
			return { MakeLink (link, text), Tags::Br };
		}

		Nodes MakeEmbedImage (const Enclosure& enclosure)
		{
			Nodes nodes;
			constexpr auto expectedMaxNodesCount = 7;
			nodes.reserve (expectedMaxNodesCount);

			const auto embedImages = XmlSettingsManager::Instance ()->property ("EmbedMediaRSSImages").toBool ();
			if (embedImages && enclosure.Type_.startsWith ("image/"_qs))
			{
				nodes.push_back (Tags::Image (enclosure.URL_));
				nodes.push_back (Tags::Br);
			}

			if (enclosure.Length_ > 0)
				nodes.push_back (TrCtx::tr ("File of type %1, size %2:")
						.arg (enclosure.Type_, Util::MakePrettySize (enclosure.Length_)));
			else
				nodes.push_back (TrCtx::tr ("File of type %1:")
						.arg (enclosure.Type_));
			nodes.push_back (Tags::Br);

			nodes.push_back (MakeLink (enclosure.URL_, QFileInfo { QUrl { enclosure.URL_ }.path () }.fileName ()));
			if (!enclosure.Lang_.isEmpty ())
			{
				nodes.push_back (Tags::Br);
				nodes.push_back (TrCtx::tr ("Specified language: %1").arg (enclosure.Lang_));
			}

			return nodes;
		}

		Tag MakeEmbedImages (const QList<Enclosure>& enclosures, const TextColor& color)
		{
			if (enclosures.isEmpty ())
				return {};

			Nodes nodes;
			nodes.reserve (enclosures.size ());
			for (const auto& enclosure : enclosures)
				nodes << WithInnerPadding (color, MakeEmbedImage (enclosure));
			return { .Name_ = "div"_qs, .Children_ = std::move (nodes) };
		}

		Tag MakeHeader (const Item& item, const TextColor& color)
		{
			auto headerStyle = R"(
						background: %1;
						color: %2;
						padding: 0em 2em 0.5em 2em;
						border: 2px none green;
						margin: 0px;
						-webkit-border-top-left-radius: 1em;
						-webkit-border-top-right-radius: 1em;
					)"_qs
					.arg (color.Bg_, color.Fg_);

			return
			{
				.Name_ = "div"_qs,
				.Attrs_ = { { "style"_qs, std::move (headerStyle) } },
				.Children_ =
						MakeItemLink (item) +
						MakePublishedInfo (item) +
						MakeCategories (item) +
						MakeComments (item) +
						MakeGeolocation (item)
			};
		}
	}

	QString ItemToHtml (const Item& item)
	{
		const auto& headerBg = GetHex (QPalette::Window);
		const auto& borderColor = headerBg;
		const auto& headerText = GetHex (QPalette::WindowText);
		const auto& alternateBg = GetHex (QPalette::AlternateBase);

		const TextColor altColor { .Fg_ = headerText, .Bg_ = alternateBg };
		const TextColor blockColor { .Fg_ = headerText, .Bg_ = headerBg };

		QString result;
		result = R"(
				<style>a { color: %2; } a.visited { color: %3 }</style>
				<div style='background: %1;
					margin: 0em 0em 0.5em 0em;
					padding: 0px;
					border: 2px solid %4;
					-webkit-border-radius: 1em;'
				>
				)"_qs
				.arg (GetHex (QPalette::Base),
						GetHex (QPalette::Link),
						GetHex (QPalette::LinkVisited),
						borderColor);

		result += MakeHeader (item, blockColor).ToHtml ();

		// Description
		result += R"(
				<div style='color: %1;
					padding-top: 0.5em;
					padding-left: 1em;
					padding-right: 1em;'>
				)"_qs
				.arg (GetHex (QPalette::Text));
		result += item.Description_;

		result += MakeEmbedImages (item.Enclosures_, blockColor).ToHtml ();

		for (const auto& entry : item.MRSSEntries_)
			result += WithInnerPadding (blockColor, MakeMRSSEntry (entry, altColor)).ToHtml ();

		result += "</div>"_qs + "</div>"_qs;

		return result;
	}
}
