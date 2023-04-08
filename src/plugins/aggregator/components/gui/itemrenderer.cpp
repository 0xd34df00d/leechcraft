/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemrenderer.h"
#include <QGuiApplication>
#include <QFileInfo>
#include <QPalette>
#include <QUrl>
#include <util/util.h>
#include <util/sll/qtutil.h>
#include "item.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	namespace
	{
		struct TrContext
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::ItemToHtml)
		};

		QString GetHex (QPalette::ColorRole role, QPalette::ColorGroup group = QPalette {}.currentColorGroup ())
		{
			return QPalette {}.color (group, role).name (QColor::HexArgb);
		}

		QString MakeLink (const QString& link, const QString& text)
		{
			const bool useExternal = XmlSettingsManager::Instance ()->property ("AlwaysUseExternalBrowser").toBool ();
			const auto& target = useExternal ? " target='_blank'"_qs : QString {};
			return "<a href='"_qs + link + '\'' + target + '>' + text + "</a>"_qs;
		}

		void AddItemLink (QString& result, const Item& item)
		{
			result += MakeLink (item.Link_, "<strong>"_qs + item.Title_ + "</strong>"_qs);
		}

		void AddPublishedInfo (QString& result, const Item& item)
		{
			if (!item.PubDate_.isValid () && item.Author_.isEmpty ())
				return;

			if (item.PubDate_.isValid () && !item.Author_.isEmpty ())
				result += TrContext::tr ("Published on %1 by %2")
						.arg (item.PubDate_.toString (), item.Author_);
			else if (item.PubDate_.isValid ())
				result += TrContext::tr ("Published on %1")
						.arg (item.PubDate_.toString ());
			else if (!item.Author_.isEmpty ())
				result += TrContext::tr ("Published by %1")
						.arg (item.Author_);

			result += "<br />"_qs;
		}

		void AddCategories (QString& result, const Item& item)
		{
			if (!item.Categories_.isEmpty ())
				result += item.Categories_.join ("; "_qs) + "<br />"_qs;
		}

		void AddComments (QString& result, const Item& item)
		{
			if (item.NumComments_ < 0 && item.CommentsPageLink_.isEmpty ())
				return;

			const auto& text = item.NumComments_ >= 0 ?
					TrContext::tr ("%n comment(s)", nullptr, item.NumComments_) :
					TrContext::tr ("View comments");
			result += item.CommentsPageLink_.isEmpty () ?
					text :
					MakeLink (item.CommentsPageLink_, text);
			result += "<br />"_qs;
		}

		void AddGeolocation (QString& result, const Item& item)
		{
			if (!static_cast<bool> (item.Latitude_) && !static_cast<bool> (item.Longitude_))
				return;

			const auto& latStr = QString::number (item.Latitude_);
			const auto& lonStr = QString::number (item.Longitude_);

			const auto& link = "https://maps.google.com/maps?f=q&source=s_q&hl=en&geocode=&q=%1+%2"_qs
					.arg (latStr, lonStr);
			const auto& text = TrContext::tr ("Geoposition:") + ' ' + latStr + ' ' + lonStr + "<br />"_qs;
			result += MakeLink (link, text);
		}

		struct TextColor
		{
			QString Fg_;
			QString Bg_;
		};

		QString GetInnerPadding (const TextColor& color)
		{
			return R"(
					<div style='background: %1;
						color: %2;
						border: 1px solid #333333;
						padding: 0.2em 2em;
						-webkit-border-radius: 1em;'>
					)"_qs
					.arg (color.Bg_, color.Fg_);
		}

		void AddEmbedImages (QString& result, const QList<Enclosure>& enclosures, const TextColor& color)
		{
			const auto embedImages = XmlSettingsManager::Instance ()->property ("EmbedMediaRSSImages").toBool ();

			const auto& divStart = GetInnerPadding (color);
			for (const auto& enclosure : enclosures)
			{
				result += divStart;

				if (embedImages && enclosure.Type_.startsWith ("image/"_qs))
					result += "<img src='%1' /><br/>"_qs
							.arg (enclosure.URL_);

				if (enclosure.Length_ > 0)
					result += TrContext::tr ("File of type %1, size %2:")
							.arg (enclosure.Type_, Util::MakePrettySize (enclosure.Length_));
				else
					result += TrContext::tr ("File of type %1:")
							.arg (enclosure.Type_);
				result += "<br />"_qs;

				result += MakeLink (enclosure.URL_, QFileInfo { QUrl { enclosure.URL_ }.path () }.fileName ());
				if (!enclosure.Lang_.isEmpty ())
					result += "<br />"_qs +
							TrContext::tr ("Specified language: %1").arg (enclosure.Lang_);

				result += "</div>"_qs;
			}
		}

		void AddHeader (QString& result, const Item& item, const TextColor& color)
		{
			result += R"(
					<div style='background: %1;
						color: %2;
						padding: 0em 2em 0.5em 2em;
						border: 2px none green;
						margin: 0px;
						-webkit-border-top-left-radius: 1em;
						-webkit-border-top-right-radius: 1em;'>
					)"_qs
					.arg (color.Bg_, color.Fg_);

			AddItemLink (result, item);
			AddPublishedInfo (result, item);
			AddCategories (result, item);
			AddComments (result, item);
			AddGeolocation (result, item);

			result += "</div>"_qs;
		}
	}

	QString ItemToHtml (const Item& item)
	{
		const auto& headerBg = GetHex (QPalette::Window);
		const auto& borderColor = headerBg;
		const auto& headerText = GetHex (QPalette::WindowText);
		const auto& alternateBg = GetHex (QPalette::AlternateBase);

		auto result = R"(
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

		AddHeader (result, item, { .Fg_ = headerText, .Bg_ = headerBg });

		// Description
		result += R"(
				<div style='color: %1;
					padding-top: 0.5em;
					padding-left: 1em;
					padding-right: 1em;'>
				)"_qs
				.arg (GetHex (QPalette::Text));
		result += item.Description_;

		AddEmbedImages (result, item.Enclosures_, { .Fg_ = headerText, .Bg_ = headerBg });

		for (const auto& entry : item.MRSSEntries_)
		{
			result += GetInnerPadding ({ .Fg_ = headerText, .Bg_ = headerBg });

			QString url = entry.URL_;

			if (entry.Medium_ == "image")
				result += TrContext::tr ("Image") + ' ';
			else if (entry.Medium_ == "audio")
				result += TrContext::tr ("Audio") + ' ';
			else if (entry.Medium_ == "video")
				result += TrContext::tr ("Video") + ' ';
			else if (entry.Medium_ == "document")
				result += TrContext::tr ("Document") + ' ';
			else if (entry.Medium_ == "executable")
				result += TrContext::tr ("Executable") + ' ';

			if (entry.Title_.isEmpty ())
				result += QString ("<a href='%1' target='_blank'>%1</a><hr />")
						.arg (url);
			else
				result += QString ("<a href='%1' target='_blank'>%2</a><hr />")
						.arg (url)
						.arg (entry.Title_);

			if (entry.Size_ > 0)
			{
				result += Util::MakePrettySize (entry.Size_);
				result += "<br />";
			}

			QString peers;
			for (const auto& pl : entry.PeerLinks_)
				peers += QString ("<li>Also available in <a href='%1'>P2P (%2)</a></li>")
						.arg (pl.Link_)
						.arg (pl.Type_);
			if (peers.size ())
			{
				result += GetInnerPadding ({ .Fg_ = headerText, .Bg_ = alternateBg });
				result += QString ("<ul>%1</ul>")
						.arg (peers);
				result += "</div>";
			}

			if (!entry.Description_.isEmpty ())
				result += QString ("%1<br />")
						.arg (entry.Description_);

			QList<int> sizes;
			int num = 0;
			for (int i = 0; i < entry.Thumbnails_.size (); ++i)
			{
				int width = entry.Thumbnails_.at (i).Width_;
				if (!width)
					break;

				if (!sizes.contains (width))
					sizes << width;
				else
				{
					bool broke = false;;
					for (int j = i + 1; j < entry.Thumbnails_.size (); ++j)
						if (entry.Thumbnails_.at (j).Width_ == sizes.at (j % sizes.size ()))
						{
							broke = true;
							break;
						}

					if (broke)
						continue;
					num = sizes.size ();
					break;
				}
			}

			if (!num || num == entry.Thumbnails_.size ())
				num = 3;

			int cur = 1;
			for (const auto& thumb : entry.Thumbnails_)
			{
				if (!thumb.Time_.isEmpty ())
					result += TrContext::tr ("<hr />Thumbnail at %1:<br />")
							.arg (thumb.Time_);
				result += QString ("<img src='%1' ")
						.arg (thumb.URL_);
				if (thumb.Width_)
					result += QString ("width='%1' ")
							.arg (thumb.Width_);
				if (thumb.Height_)
					result += QString ("height='%1' ")
							.arg (thumb.Height_);
				result += "/>";

				if (num && cur < num)
					++cur;
				else
				{
					result += "<br />";
					cur = 1;
				}
			}

			result += "<hr />";

			if (!entry.Keywords_.isEmpty ())
				result += TrContext::tr ("<strong>Keywords:</strong> <em>%1</em><br />")
						.arg (entry.Keywords_);

			if (!entry.Lang_.isEmpty ())
				result += TrContext::tr ("<strong>Language:</strong> %1<br />")
						.arg (entry.Lang_);

			if (entry.Expression_ == "sample")
				result += TrContext::tr ("Sample");
			else if (entry.Expression_ == "nonstop")
				result += TrContext::tr ("Continuous stream");
			else
				result += TrContext::tr ("Full version");
			result += "<br />";

			QString scenes;
			for (const auto& sc : entry.Scenes_)
			{
				QString current;
				if (!sc.Title_.isEmpty ())
					current += TrContext::tr ("Title: %1<br />")
							.arg (sc.Title_);
				if (!sc.StartTime_.isEmpty ())
					current += TrContext::tr ("Start time: %1<br />")
							.arg (sc.StartTime_);
				if (!sc.EndTime_.isEmpty ())
					current += TrContext::tr ("End time: %1<br />")
							.arg (sc.EndTime_);
				if (!sc.Description_.isEmpty ())
					current += QString ("%1<br />")
							.arg (sc.Description_);

				if (!current.isEmpty ())
					scenes += QString ("<li>%1</li>")
							.arg (current);
			}

			if (scenes.size ())
			{
				result += TrContext::tr ("<strong>Scenes:</strong>");
				result += GetInnerPadding ({ .Fg_ = headerText, .Bg_ = alternateBg });
				result += QString ("<ul>%1</ul>")
						.arg (scenes);
				result += "</div>";
			}

			if (entry.Views_)
				result += TrContext::tr ("<strong>Views:</strong> %1")
						.arg (entry.Views_);
			if (entry.Favs_)
				result += TrContext::tr ("<strong>Added to favorites:</strong> %n time(s)",
						"", entry.Favs_);
			if (entry.RatingAverage_)
				result += TrContext::tr ("<strong>Average rating:</strong> %1")
						.arg (entry.RatingAverage_);
			if (entry.RatingCount_)
				result += TrContext::tr ("<strong>Number of marks:</strong> %1")
						.arg (entry.RatingCount_);
			if (entry.RatingMin_)
				result += TrContext::tr ("<strong>Minimal rating:</strong> %1")
						.arg (entry.RatingMin_);
			if (entry.RatingMax_)
				result += TrContext::tr ("<strong>Maximal rating:</strong> %1")
						.arg (entry.RatingMax_);

			if (!entry.Tags_.isEmpty ())
				result += TrContext::tr ("<strong>User tags:</strong> %1")
						.arg (entry.Tags_);

			QString tech;
			if (entry.Duration_)
				tech += TrContext::tr ("<li><strong>Duration:</strong> %1</li>")
						.arg (entry.Channels_);
			if (entry.Channels_)
				tech += TrContext::tr ("<li><strong>Channels:</strong> %1</li>")
						.arg (entry.Channels_);
			if (entry.Width_ &&
					entry.Height_)
				tech += TrContext::tr ("<li><strong>Size:</strong> %1x%2</li>")
						.arg (entry.Width_)
						.arg (entry.Height_);
			if (entry.Bitrate_)
				tech += TrContext::tr ("<li><strong>Bitrate:</strong> %1 kbps</li>")
						.arg (entry.Bitrate_);
			if (entry.Framerate_)
				tech += TrContext::tr ("<li><strong>Framerate:</strong> %1</li>")
						.arg (entry.Framerate_);
			if (entry.SamplingRate_)
				tech += TrContext::tr ("<li><strong>Sampling rate:</strong> %1</li>")
						.arg (entry.SamplingRate_);
			if (!entry.Type_.isEmpty ())
				tech += TrContext::tr ("<li><strong>MIME type:</strong> %1</li>")
						.arg (entry.Type_);

			if (!tech.isEmpty ())
			{
				result += TrContext::tr ("<strong>Technical information:</strong>");
				result += GetInnerPadding ({ .Fg_ = headerText, .Bg_ = alternateBg });
				result += QString ("<ul>%1</ul>")
						.arg (tech);
				result += "</div>";
			}

			if (!entry.Rating_.isEmpty () &&
					!entry.RatingScheme_.isEmpty ())
				result += TrContext::tr ("<strong>Rating:</strong> %1 (according to %2 scheme)<br />")
						.arg (entry.Rating_)
						.arg (entry.RatingScheme_.mid (4));

			QMap<QString, QString> comments;
			for (const auto& cm : entry.Comments_)
				comments [cm.Type_] += QString ("<li>%1</li>")
						.arg (cm.Comment_);

			QStringList cmTypes = comments.keys ();
			for (const auto& type : cmTypes)
			{
				result += QString ("<strong>%1:</strong>")
						.arg (type);
				result += GetInnerPadding ({ .Fg_ = headerText, .Bg_ = alternateBg });
				result += QString ("<ul>%1</ul>")
						.arg (comments [type]);
				result += "</div>";
			}

			if (!entry.CopyrightURL_.isEmpty ())
			{
				if (!entry.CopyrightText_.isEmpty ())
					result += TrContext::tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%2</a><br />")
							.arg (entry.CopyrightURL_)
							.arg (entry.CopyrightText_);
				else
					result += TrContext::tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%1</a><br />")
							.arg (entry.CopyrightURL_);
			}
			else if (!entry.CopyrightText_.isEmpty ())
				result += TrContext::tr ("<strong>Copyright:</strong> %1<br />")
						.arg (entry.CopyrightText_);

			QString credits;
			for (const auto& cr : entry.Credits_)
				if (!cr.Role_.isEmpty ())
					credits += QString ("<li>%1: %2</li>")
							.arg (cr.Role_)
							.arg (cr.Who_);

			if (!credits.isEmpty ())
			{
				result += TrContext::tr ("<strong>Credits:</strong>");
				result += GetInnerPadding ({ .Fg_ = headerText, .Bg_ = alternateBg });
				result += QString ("<ul>%1</ul>")
						.arg (credits);
				result += "</div>";
			}

			result += "</div>";
		}

		result += "</div>";
		result += "</div>";

		return result;
	}
}
