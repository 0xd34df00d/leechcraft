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
#include <util/sll/util.h>
#include <util/sll/visitor.h>
#include "item.h"
#include "xmlsettingsmanager.h"
#include "xmlnode.h"

namespace LC::Aggregator
{
	namespace
	{
		QString GetHex (QPalette::ColorRole role, QPalette::ColorGroup group = QPalette {}.currentColorGroup ())
		{
			return QPalette {}.color (group, role).name (QColor::HexRgb);
		}

		struct Writer
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::ItemToHtml)
		};

		Tag MakeLink (const QString& target, Node contents)
		{
			TagAttrs attrs { { "href"_qs, target } };
			if (XmlSettingsManager::Instance ()->property ("AlwaysUseExternalBrowser").toBool ())
				attrs.push_back ({ "target"_qs, "blank"_qs });

			return { .Name_ = "a", .Attrs_ = std::move (attrs), .Children_ = { std::move (contents) } };
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
				info = Writer::tr ("Published on %1 by %2")
						.arg (item.PubDate_.toString (), item.Author_);
			else if (item.PubDate_.isValid ())
				info = Writer::tr ("Published on %1")
						.arg (item.PubDate_.toString ());
			else if (!item.Author_.isEmpty ())
				info = Writer::tr ("Published by %1")
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
					Writer::tr ("%n comment(s)", nullptr, item.NumComments_) :
					Writer::tr ("View comments");
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
			const auto& text = Writer::tr ("Geoposition:") + ' ' + latStr + ' ' + lonStr;
			return { MakeLink (link, text), Tags::Br };
		}

		struct TextColor
		{
			QString Fg_;
			QString Bg_;
		};

		Tag WithInnerPadding (const TextColor& color, Nodes&& children)
		{
			auto blockStyle = R"(
					background: %1;
					color: %2;
					border: 1px solid #333333;
					padding: 0.2em 2em;
					-webkit-border-radius: 1em;
					)"_qs
					.arg (color.Bg_, color.Fg_);
			return
			{
				.Name_ = "div"_qs,
				.Attrs_ = { { "style"_qs, std::move (blockStyle) } },
				.Children_ = std::move (children),
			};
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
				nodes.push_back (Writer::tr ("File of type %1, size %2:")
						.arg (enclosure.Type_, Util::MakePrettySize (enclosure.Length_)));
			else
				nodes.push_back (Writer::tr ("File of type %1:")
						.arg (enclosure.Type_));
			nodes.push_back (Tags::Br);

			nodes.push_back (MakeLink (enclosure.URL_, QFileInfo { QUrl { enclosure.URL_ }.path () }.fileName ()));
			if (!enclosure.Lang_.isEmpty ())
			{
				nodes.push_back (Tags::Br);
				nodes.push_back (Writer::tr ("Specified language: %1").arg (enclosure.Lang_));
			}

			return nodes;
		}

		Tag MakeEmbedImages (const QList<Enclosure>& enclosures, const TextColor& color)
		{
			Nodes nodes;
			nodes.reserve (enclosures.size ());
			for (const auto& enclosure : enclosures)
				nodes << WithInnerPadding (color, MakeEmbedImage (enclosure));
			return { .Name_ = "div"_qs, .Children_ = std::move (nodes) };
		}

		QString GetMRSSMedium (const QString& medium)
		{
			if (medium == "image"_ql)
				return Writer::tr ("Image");
			if (medium == "audio"_ql)
				return Writer::tr ("Audio");
			if (medium == "video"_ql)
				return Writer::tr ("Video");
			if (medium == "document"_ql)
				return Writer::tr ("Document");
			if (medium == "executable"_ql)
				return Writer::tr ("Executable");

			return medium;
		}

		Nodes MakeMRSSHeader (const MRSSEntry& entry)
		{
			Nodes nodes
			{
				GetMRSSMedium (entry.Medium_),
				" "_qs,
				MakeLink (entry.URL_, entry.Title_.isEmpty () ? entry.URL_ : entry.Title_),
				Tags::Br,
			};

			if (entry.Size_ > 0)
			{
				nodes.push_back (Util::MakePrettySize (entry.Size_));
				nodes.push_back (Tags::Br);
			}

			if (!entry.Tags_.isEmpty ())
			{
				nodes.push_back (Writer::tr ("Tags") + ' ' + entry.Tags_);
				nodes.push_back (Tags::Br);
			}

			if (!entry.Rating_.isEmpty ())
			{
				constexpr auto urnPrefixLength = 4; // "urn:"
				const auto& scheme = entry.RatingScheme_.mid (urnPrefixLength);
				auto rating = scheme.isEmpty () ?
						entry.Rating_ :
						Writer::tr ("%1 (as per %2)", "<rating> (as per <rating scheme>)")
							.arg (entry.Rating_, scheme);
				nodes.push_back (Writer::tr ("Rating") + ": "_qs + rating);
				nodes.push_back (Tags::Br);
			}

			return nodes;
		}

		Nodes MakeMRSSPeerLinks (const QList<MRSSPeerLink>& links, const TextColor& color)
		{
			if (links.isEmpty ())
				return {};

			Nodes linksDescrs { links.size () };
			for (const auto& link : links)
				linksDescrs << Tag { .Name_ = "li"_qs, .Children_ = { MakeLink (link.Link_, link.Type_) } };

			Nodes block
			{
				Writer::tr ("Also available as:"),
				Tag { .Name_ = "ul"_qs, .Children_ = std::move (linksDescrs) },
			};
			return { WithInnerPadding (color, std::move (block)) };
		}

		Nodes MakeMRSSDescription (const MRSSEntry& entry)
		{
			return { entry.Description_, Tags::Br };
		}

		Nodes MakeMRSSThumbnails (const QList<MRSSThumbnail>& thumbs)
		{
			if (thumbs.isEmpty ())
				return {};

			constexpr auto nodesPerThumb = 2;
			Nodes nodes { thumbs.size () * nodesPerThumb + 1 };
			for (const auto& thumb : thumbs)
			{
				TagAttrs attrs { { "src"_qs, thumb.URL_ } };
				if (thumb.Width_)
					attrs.push_back ({ "width"_qs, QString::number (thumb.Width_) });
				if (thumb.Height_)
					attrs.push_back ({ "height"_qs, QString::number (thumb.Height_) });
				if (!thumb.Time_.isEmpty ())
					attrs.push_back ({ "alt"_qs, Writer::tr ("Thumbnail at %1").arg (thumb.Time_) });
				nodes.push_back (Tag { .Name_ = "img", .Attrs_ = std::move (attrs) });
			}
			nodes.push_back (Tags::Br);
			return nodes;
		}

		Nodes MakeMRSSField (const QString& text, const QString& contents)
		{
			if (contents.isEmpty ())
				return {};

			return
			{
				Tag::WithText ("strong"_qs, text + ':'),
				' ' + contents,
				Tags::Br
			};
		}

		Nodes MakeMRSSExpression (const QString& expression)
		{
			QString label;
			if (expression == "sample")
				label = Writer::tr ("Sample");
			else if (expression == "nonstop")
				label = Writer::tr ("Continuous stream");
			else
				label = Writer::tr ("Full version");

			return { std::move (label), Tags::Br };
		}

		Nodes MakeMRSSScene (const MRSSScene& scene)
		{
			const QVector<QPair<QString, QString>> rows
			{
				{ Writer::tr ("Title"), scene.Title_ },
				{ Writer::tr ("Start time"), scene.StartTime_ },
				{ Writer::tr ("End time"), scene.EndTime_ },
			};

			Nodes nodes;
			nodes.reserve ((rows.size () + 1) * 2);

			for (const auto& [title, contents] : rows)
			{
				if (contents.isEmpty ())
					continue;

				nodes.push_back (title + ": "_qs + contents);
				nodes.push_back (Tags::Br);
			}

			if (!scene.Description_.isEmpty ())
			{
				nodes.push_back (scene.Description_);
				nodes.push_back (Tags::Br);
			}

			return nodes;
		}

		Nodes MakeMRSSScenes (const QList<MRSSScene>& scenes, const TextColor& color)
		{
			if (scenes.isEmpty ())
				return {};

			Nodes nodes;
			for (const auto& scene : scenes)
				nodes.push_back (Tag { .Name_ = "li"_qs, .Children_ = MakeMRSSScene (scene) });

			return
			{
				Tag::WithText ("strong"_qs, Writer::tr ("Scenes:")),
				WithInnerPadding (color, { Tag { .Name_ = "ul"_qs, .Children_ = std::move (nodes)} }),
			};
		}

		Nodes MakeMRSSStats (const MRSSEntry& entry, const TextColor& color)
		{
			const QVector<QPair<QString, int>> rows
			{
				{ Writer::tr ("Views"), entry.Views_ },
				{ Writer::tr ("Bookmarks"), entry.Favs_ },
				{ Writer::tr ("Averate rating"), entry.RatingAverage_ },
				{ Writer::tr ("Votes"), entry.RatingCount_ },
				{ Writer::tr ("Minimal rating"), entry.RatingMin_ },
				{ Writer::tr ("Maximal rating"), entry.RatingMax_ },
			};

			Nodes nodes;
			nodes.reserve (rows.size () * 2);
			for (const auto& [label, value] : rows)
			{
				if (!value)
					continue;

				nodes.push_back (label + ": "_qs + QString::number (value));
				nodes.push_back (Tags::Br);
			}

			return
			{
				Tag::WithText ("strong"_qs, Writer::tr ("Statistics:")),
				WithInnerPadding (color, std::move (nodes)),
			};
		}

		template<typename T>
		requires std::integral<T> || std::floating_point<T>
		QPair<QString, QString> IntRow (const QString& name, T value)
		{
			return { name, value ? QString::number (value) : QString {} };
		}

		Nodes MakeMRSSTechInfo (const MRSSEntry& entry, const TextColor& color)
		{
			QString size;
			if (entry.Width_ && entry.Height_)
				size = QString::number (entry.Width_) + 'x' + QString::number (entry.Height_);
			const QVector<QPair<QString, QString>> rows
			{
				IntRow (Writer::tr ("Duration"), entry.Duration_),
				IntRow (Writer::tr ("Channels"), entry.Channels_),
				{ Writer::tr ("Size"), size },
				{ Writer::tr ("Bitrate"), entry.Bitrate_ ? Writer::tr ("%1 kbps").arg (entry.Bitrate_) : QString {} },
				IntRow (Writer::tr ("Framerate"), entry.Framerate_),
				IntRow (Writer::tr ("Sampling rate"), entry.SamplingRate_),
				{ Writer::tr ("MIME type"), entry.Type_ },
			};

			Nodes nodes;
			nodes.reserve (rows.size ());
			for (const auto& [label, value] : rows)
			{
				if (value.isEmpty ())
					continue;

				nodes.push_back (Tag { .Name_ = "li"_qs, .Children_ = { label + ": "_qs + value } });
			}

			if (nodes.isEmpty ())
				return {};

			return
			{
				Tag::WithText ("strong"_qs, Writer::tr ("Technical information:")),
				WithInnerPadding (color, { Tag { .Name_ = "ul"_qs, .Children_ = std::move (nodes) } }),
			};
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

		Nodes MakeMRSSEntry (const MRSSEntry& entry, const TextColor& color)
		{
			return MakeMRSSHeader (entry) +
					MakeMRSSPeerLinks (entry.PeerLinks_, color) +
					MakeMRSSDescription (entry) +
					MakeMRSSThumbnails (entry.Thumbnails_) +
					MakeMRSSField (Writer::tr ("Keywords"), entry.Keywords_) +
					MakeMRSSField (Writer::tr ("Language"), entry.Lang_) +
					MakeMRSSExpression (entry.Expression_) +
					MakeMRSSScenes (entry.Scenes_, color) +
					MakeMRSSStats (entry, color) +
					MakeMRSSTechInfo (entry, color);
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
		{
			result += WithInnerPadding (blockColor, MakeMRSSEntry (entry, altColor)).ToHtml ();
			/*
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
					result += Writer::tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%2</a><br />")
							.arg (entry.CopyrightURL_)
							.arg (entry.CopyrightText_);
				else
					result += Writer::tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%1</a><br />")
							.arg (entry.CopyrightURL_);
			}
			else if (!entry.CopyrightText_.isEmpty ())
				result += Writer::tr ("<strong>Copyright:</strong> %1<br />")
						.arg (entry.CopyrightText_);

			QString credits;
			for (const auto& cr : entry.Credits_)
				if (!cr.Role_.isEmpty ())
					credits += QString ("<li>%1: %2</li>")
							.arg (cr.Role_)
							.arg (cr.Who_);

			if (!credits.isEmpty ())
			{
				result += Writer::tr ("<strong>Credits:</strong>");
				result += GetInnerPadding ({ .Fg_ = headerText, .Bg_ = alternateBg });
				result += QString ("<ul>%1</ul>")
						.arg (credits);
				result += "</div>";
			}
			*/
		}

		result += "</div>"_qs + "</div>"_qs;

		return result;
	}
}
