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
	}

	QString ItemToHtml (const Item& item)
	{
		const auto& headerBg = GetHex (QPalette::Window);
		const auto& borderColor = headerBg;
		const auto& headerText = GetHex (QPalette::WindowText);
		const auto& alternateBg = GetHex (QPalette::AlternateBase);

		QString firstStartBox = "<div style='background: %1; "
								"color: COLOR; "
								"padding-left: 2em; "
								"padding-right: 2em; "
								"padding-bottom: 0.5em;"
								"border: 2px none green; "
								"margin: 0px; "
								"-webkit-border-top-left-radius: 1em; "
								"-webkit-border-top-right-radius: 1em;'>";
		firstStartBox.replace ("COLOR", headerText);

		bool linw = XmlSettingsManager::Instance ()->
				property ("AlwaysUseExternalBrowser").toBool ();

		QString result = QString (
				"<style>a { color: %2; } a.visited { color: %3 }</style>"
				"<div style='background: %1; "
				"margin-top: 0em; "
				"margin-left: 0em; "
				"margin-right: 0em; "
				"margin-bottom: 0.5 em; "
				"padding: 0px; "
				"border: 2px solid %4; "
				"-webkit-border-radius: 1em;'>")
				.arg (GetHex (QPalette::Base))
				.arg (GetHex (QPalette::Link))
				.arg (GetHex (QPalette::LinkVisited))
				.arg (borderColor);

		QString inpad = QString ("<div style='background: %1; "
								 "color: %2; "
								 "border: 1px solid #333333; "
								 "padding-top: 0.2em; "
								 "padding-bottom: 0.2em; "
								 "padding-left: 2em; "
								 "padding-right: 2em;"
								 "-webkit-border-radius: 1em;'>");

		result += firstStartBox.arg (headerBg);

		// Link
		result += ("<a href='" +
				item.Link_ +
				"'");
		if (linw)
			result += " target='_blank'";
		result += QString (">");
		result += (QString ("<strong>") +
				item.Title_ +
				"</strong>" +
				"</a><br />");

		// Publication date and author
		if (item.PubDate_.isValid () && !item.Author_.isEmpty ())
			result += TrContext::tr ("Published on %1 by %2")
					.arg (item.PubDate_.toString ())
					.arg (item.Author_) +
					"<br />";
		else if (item.PubDate_.isValid ())
			result += TrContext::tr ("Published on %1")
					.arg (item.PubDate_.toString ()) +
					"<br />";
		else if (!item.Author_.isEmpty ())
			result += TrContext::tr ("Published by %1")
					.arg (item.Author_) +
					"<br />";

		// Categories
		if (item.Categories_.size ())
			result += item.Categories_.join ("; ") +
					"<br />";

		// Comments stuff
		if (item.NumComments_ >= 0 && !item.CommentsPageLink_.isEmpty ())
			result += TrContext::tr ("%n comment(s), <a href='%1'%2>view them</a><br />",
					"", item.NumComments_)
					.arg (item.CommentsPageLink_)
					.arg (linw ? " target='_blank'" : "");
		else if (item.NumComments_ >= 0)
			result += TrContext::tr ("%n comment(s)", "", item.NumComments_) +
					"<br />";
		else if (!item.CommentsPageLink_.isEmpty ())
			result += TrContext::tr ("<a href='%1'%2>View comments</a><br />")
					.arg (item.CommentsPageLink_)
					.arg (linw ? " target='_blank'" : "");

		if (item.Latitude_ ||
				item.Longitude_)
		{
			QString link = QString ("http://maps.google.com/maps"
									"?f=q&source=s_q&hl=en&geocode=&q=%1+%2")
					.arg (item.Latitude_)
					.arg (item.Longitude_);
			result += TrContext::tr ("Geoposition: <a href='%3'%4 title='Google Maps'>%1 %2</a><br />")
					.arg (item.Latitude_)
					.arg (item.Longitude_)
					.arg (link)
					.arg (linw ? " target='_blank'" : "");
		}

		// Description
		result += QString ("</div><div style='color: %1;"
						   "padding-top: 0.5em; "
						   "padding-left: 1em; "
						   "padding-right: 1em;'>")
				.arg (GetHex (QPalette::Text));
		result += item.Description_;

		const auto embedImages = XmlSettingsManager::Instance ()->
				property ("EmbedMediaRSSImages").toBool ();
		for (const auto& enclosure : item.Enclosures_)
		{
			result += inpad.arg (headerBg)
					.arg (headerText);

			if (embedImages && enclosure.Type_.startsWith ("image/"))
				result += QString ("<img src='%1' /><br/>")
						.arg (enclosure.URL_);

			if (enclosure.Length_ > 0)
				result += TrContext::tr ("File of type %1, size %2:<br />")
						.arg (enclosure.Type_)
						.arg (Util::MakePrettySize (enclosure.Length_));
			else
				result += TrContext::tr ("File of type %1 and unknown length:<br />")
						.arg (enclosure.Type_);

			result += QString ("<a href='%1'>%2</a>")
					.arg (enclosure.URL_)
					.arg (QFileInfo (QUrl (enclosure.URL_).path ()).fileName ());
			if (!enclosure.Lang_.isEmpty ())
				result += TrContext::tr ("<br />Specified language: %1")
						.arg (enclosure.Lang_);
			result += "</div>";
		}

		for (QList<MRSSEntry>::const_iterator entry = item.MRSSEntries_.begin (),
				endEntry = item.MRSSEntries_.end (); entry != endEntry; ++entry)
		{
			result += inpad.arg (headerBg)
					.arg (headerText);

			QString url = entry->URL_;

			if (entry->Medium_ == "image")
				result += TrContext::tr ("Image") + ' ';
			else if (entry->Medium_ == "audio")
				result += TrContext::tr ("Audio") + ' ';
			else if (entry->Medium_ == "video")
				result += TrContext::tr ("Video") + ' ';
			else if (entry->Medium_ == "document")
				result += TrContext::tr ("Document") + ' ';
			else if (entry->Medium_ == "executable")
				result += TrContext::tr ("Executable") + ' ';

			if (entry->Title_.isEmpty ())
				result += QString ("<a href='%1' target='_blank'>%1</a><hr />")
						.arg (url);
			else
				result += QString ("<a href='%1' target='_blank'>%2</a><hr />")
						.arg (url)
						.arg (entry->Title_);

			if (entry->Size_ > 0)
			{
				result += Util::MakePrettySize (entry->Size_);
				result += "<br />";
			}

			QString peers;
			for (const auto& pl : entry->PeerLinks_)
				peers += QString ("<li>Also available in <a href='%1'>P2P (%2)</a></li>")
						.arg (pl.Link_)
						.arg (pl.Type_);
			if (peers.size ())
			{
				result += inpad.arg (alternateBg)
						.arg (headerText);
				result += QString ("<ul>%1</ul>")
						.arg (peers);
				result += "</div>";
			}

			if (!entry->Description_.isEmpty ())
				result += QString ("%1<br />")
						.arg (entry->Description_);

			QList<int> sizes;
			int num = 0;
			for (int i = 0; i < entry->Thumbnails_.size (); ++i)
			{
				int width = entry->Thumbnails_.at (i).Width_;
				if (!width)
					break;

				if (!sizes.contains (width))
					sizes << width;
				else
				{
					bool broke = false;;
					for (int j = i + 1; j < entry->Thumbnails_.size (); ++j)
						if (entry->Thumbnails_.at (j).Width_ == sizes.at (j % sizes.size ()))
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

			if (!num || num == entry->Thumbnails_.size ())
				num = 3;

			int cur = 1;
			for (const auto& thumb : entry->Thumbnails_)
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

			if (!entry->Keywords_.isEmpty ())
				result += TrContext::tr ("<strong>Keywords:</strong> <em>%1</em><br />")
						.arg (entry->Keywords_);

			if (!entry->Lang_.isEmpty ())
				result += TrContext::tr ("<strong>Language:</strong> %1<br />")
						.arg (entry->Lang_);

			if (entry->Expression_ == "sample")
				result += TrContext::tr ("Sample");
			else if (entry->Expression_ == "nonstop")
				result += TrContext::tr ("Continuous stream");
			else
				result += TrContext::tr ("Full version");
			result += "<br />";

			QString scenes;
			for (const auto& sc : entry->Scenes_)
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
				result += inpad.arg (alternateBg)
						.arg (headerText);
				result += QString ("<ul>%1</ul>")
						.arg (scenes);
				result += "</div>";
			}

			if (entry->Views_)
				result += TrContext::tr ("<strong>Views:</strong> %1")
						.arg (entry->Views_);
			if (entry->Favs_)
				result += TrContext::tr ("<strong>Added to favorites:</strong> %n time(s)",
						"", entry->Favs_);
			if (entry->RatingAverage_)
				result += TrContext::tr ("<strong>Average rating:</strong> %1")
						.arg (entry->RatingAverage_);
			if (entry->RatingCount_)
				result += TrContext::tr ("<strong>Number of marks:</strong> %1")
						.arg (entry->RatingCount_);
			if (entry->RatingMin_)
				result += TrContext::tr ("<strong>Minimal rating:</strong> %1")
						.arg (entry->RatingMin_);
			if (entry->RatingMax_)
				result += TrContext::tr ("<strong>Maximal rating:</strong> %1")
						.arg (entry->RatingMax_);

			if (!entry->Tags_.isEmpty ())
				result += TrContext::tr ("<strong>User tags:</strong> %1")
						.arg (entry->Tags_);

			QString tech;
			if (entry->Duration_)
				tech += TrContext::tr ("<li><strong>Duration:</strong> %1</li>")
						.arg (entry->Channels_);
			if (entry->Channels_)
				tech += TrContext::tr ("<li><strong>Channels:</strong> %1</li>")
						.arg (entry->Channels_);
			if (entry->Width_ &&
					entry->Height_)
				tech += TrContext::tr ("<li><strong>Size:</strong> %1x%2</li>")
						.arg (entry->Width_)
						.arg (entry->Height_);
			if (entry->Bitrate_)
				tech += TrContext::tr ("<li><strong>Bitrate:</strong> %1 kbps</li>")
						.arg (entry->Bitrate_);
			if (entry->Framerate_)
				tech += TrContext::tr ("<li><strong>Framerate:</strong> %1</li>")
						.arg (entry->Framerate_);
			if (entry->SamplingRate_)
				tech += TrContext::tr ("<li><strong>Sampling rate:</strong> %1</li>")
						.arg (entry->SamplingRate_);
			if (!entry->Type_.isEmpty ())
				tech += TrContext::tr ("<li><strong>MIME type:</strong> %1</li>")
						.arg (entry->Type_);

			if (!tech.isEmpty ())
			{
				result += TrContext::tr ("<strong>Technical information:</strong>");
				result += inpad.arg (alternateBg)
						.arg (headerText);
				result += QString ("<ul>%1</ul>")
						.arg (tech);
				result += "</div>";
			}

			if (!entry->Rating_.isEmpty () &&
					!entry->RatingScheme_.isEmpty ())
				result += TrContext::tr ("<strong>Rating:</strong> %1 (according to %2 scheme)<br />")
						.arg (entry->Rating_)
						.arg (entry->RatingScheme_.mid (4));

			QMap<QString, QString> comments;
			for (const auto& cm : entry->Comments_)
				comments [cm.Type_] += QString ("<li>%1</li>")
						.arg (cm.Comment_);

			QStringList cmTypes = comments.keys ();
			for (const auto& type : cmTypes)
			{
				result += QString ("<strong>%1:</strong>")
						.arg (type);
				result += inpad.arg (alternateBg)
						.arg (headerText);
				result += QString ("<ul>%1</ul>")
						.arg (comments [type]);
				result += "</div>";
			}

			if (!entry->CopyrightURL_.isEmpty ())
			{
				if (!entry->CopyrightText_.isEmpty ())
					result += TrContext::tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%2</a><br />")
							.arg (entry->CopyrightURL_)
							.arg (entry->CopyrightText_);
				else
					result += TrContext::tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%1</a><br />")
							.arg (entry->CopyrightURL_);
			}
			else if (!entry->CopyrightText_.isEmpty ())
				result += TrContext::tr ("<strong>Copyright:</strong> %1<br />")
						.arg (entry->CopyrightText_);

			QString credits;
			for (const auto& cr : entry->Credits_)
				if (!cr.Role_.isEmpty ())
					credits += QString ("<li>%1: %2</li>")
							.arg (cr.Role_)
							.arg (cr.Who_);

			if (!credits.isEmpty ())
			{
				result += TrContext::tr ("<strong>Credits:</strong>");
				result += inpad.arg (alternateBg)
						.arg (headerText);
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
