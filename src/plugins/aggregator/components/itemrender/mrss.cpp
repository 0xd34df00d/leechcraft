/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mrss.h"
#include <util/sll/qtutil.h>
#include "../../item.h"
#include "utils.h"

namespace LC::Aggregator
{
	using Nodes = Util::Nodes;
	using Tag = Util::Tag;
	using TagAttrs = Util::TagAttrs;
	namespace Tags = Util::Tags;

	namespace
	{
		QString GetMRSSMedium (const QString& medium)
		{
			if (medium == "image"_ql)
				return TrCtx::tr ("Image");
			if (medium == "audio"_ql)
				return TrCtx::tr ("Audio");
			if (medium == "video"_ql)
				return TrCtx::tr ("Video");
			if (medium == "document"_ql)
				return TrCtx::tr ("Document");
			if (medium == "executable"_ql)
				return TrCtx::tr ("Executable");

			return medium;
		}

		Nodes MakeField (const QString& label, const QString& contents)
		{
			return contents.isEmpty () ?
					Nodes {} :
					Nodes { label + ": "_qs + contents, Tags::Br };
		}

		Nodes MakeField (const QString& label, Nodes&& contents)
		{
			return contents.isEmpty () ?
					Nodes {} :
					(label + ": "_qs) + std::move (contents) + Tags::Br;
		}

		template<typename T>
		concept QStringNumber = requires { QString::number (T {}); };

		Nodes MakeField (const QString& label, QStringNumber auto value)
		{
			if (!value)
				return {};

			return MakeField (label, QString::number (value));
		}

		Nodes MakeRating (const MRSSEntry& entry)
		{
			if (entry.Rating_.isEmpty ())
				return {};

			constexpr auto urnPrefixLength = 4; // "urn:"
			const auto& scheme = entry.RatingScheme_.mid (urnPrefixLength);
			auto rating = scheme.isEmpty () ?
					entry.Rating_ :
					TrCtx::tr ("%1 (as per %2)", "<rating> (as per <rating scheme>)")
							.arg (entry.Rating_, scheme);
			return MakeField (TrCtx::tr ("Rating"), rating);
		}

		Nodes MakeHeader (const MRSSEntry& entry)
		{
			Nodes nodes
			{
				GetMRSSMedium (entry.Medium_),
				" "_qs,
				MakeLink (entry.URL_, entry.Title_.isEmpty () ? entry.URL_ : entry.Title_),
				Tags::Br,
			};

			nodes += MakeField (TrCtx::tr ("Size"), entry.Size_);
			nodes += MakeField (TrCtx::tr ("Tags"), entry.Tags_);
			nodes += MakeField (TrCtx::tr ("Keywords"), entry.Keywords_);
			nodes += MakeField (TrCtx::tr ("Language"), entry.Lang_);
			nodes += MakeRating (entry);

			return nodes;
		}

		Nodes MakePeerLinks (const QList<MRSSPeerLink>& links, const TextColor& color)
		{
			if (links.isEmpty ())
				return {};

			Nodes linksDescrs { links.size () };
			for (const auto& link : links)
				linksDescrs << Tags::Li ({ MakeLink (link.Link_, link.Type_) });

			Nodes block
			{
				TrCtx::tr ("Also available as:"),
				Tags::Ul (std::move (linksDescrs)),
			};
			return { WithInnerPadding (color, std::move (block)) };
		}

		Nodes MakeDescription (const MRSSEntry& entry)
		{
			return { entry.Description_, Tags::Br };
		}

		Nodes MakeThumbnails (const QList<MRSSThumbnail>& thumbs)
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
					attrs.push_back ({ "alt"_qs, TrCtx::tr ("Thumbnail at %1").arg (thumb.Time_) });
				nodes.push_back (Tag { .Name_ = "img", .Attrs_ = std::move (attrs) });
			}
			nodes.push_back (Tags::Br);
			return nodes;
		}

		Nodes MakeExpression (const QString& expression)
		{
			QString label;
			if (expression == "sample"_ql)
				label = TrCtx::tr ("Sample");
			else if (expression == "nonstop"_ql)
				label = TrCtx::tr ("Continuous stream");
			else
				label = TrCtx::tr ("Full version");

			return { std::move (label), Tags::Br };
		}

		Nodes MakeScene (const MRSSScene& scene)
		{
			const QVector<QPair<QString, QString>> rows
			{
				{ TrCtx::tr ("Title"), scene.Title_ },
				{ TrCtx::tr ("Start time"), scene.StartTime_ },
				{ TrCtx::tr ("End time"), scene.EndTime_ },
			};

			Nodes nodes;
			nodes.reserve ((rows.size () + 1) * 2);

			for (const auto& [label, contents] : rows)
				nodes += MakeField (label, contents);

			if (!scene.Description_.isEmpty ())
			{
				nodes.push_back (scene.Description_);
				nodes.push_back (Tags::Br);
			}

			return nodes;
		}

		Nodes MakeSubblock (const QString& header, const TextColor& color, Nodes&& children)
		{
			return
			{
				Tag::WithText ("strong"_qs, header + ':'),
				WithInnerPadding (color, std::move (children)),
			};
		}

		Nodes MakeScenes (const QList<MRSSScene>& scenes, const TextColor& color)
		{
			if (scenes.isEmpty ())
				return {};

			Nodes nodes;
			for (const auto& scene : scenes)
				nodes.push_back (Tags::Li (MakeScene (scene)));

			return MakeSubblock (TrCtx::tr ("Scenes"), color, { Tags::Ul (std::move (nodes)) });
		}

		Nodes MakeStats (const MRSSEntry& entry, const TextColor& color)
		{
			const QVector<QPair<QString, int>> rows
			{
				{ TrCtx::tr ("Views"), entry.Views_ },
				{ TrCtx::tr ("Bookmarks"), entry.Favs_ },
				{ TrCtx::tr ("Averate rating"), entry.RatingAverage_ },
				{ TrCtx::tr ("Votes"), entry.RatingCount_ },
				{ TrCtx::tr ("Minimal rating"), entry.RatingMin_ },
				{ TrCtx::tr ("Maximal rating"), entry.RatingMax_ },
			};

			Nodes nodes;
			nodes.reserve (rows.size () * 2);
			for (const auto& [label, value] : rows)
				nodes += MakeField (label, value);

			if (nodes.isEmpty ())
				return {};

			return MakeSubblock (TrCtx::tr ("Statistics"), color, std::move (nodes));
		}

		QPair<QString, QString> IntRow (const QString& name, QStringNumber auto value)
		{
			return { name, value ? QString::number (value) : QString {} };
		}

		Nodes MakeTechInfo (const MRSSEntry& entry, const TextColor& color)
		{
			QString size;
			if (entry.Width_ && entry.Height_)
				size = QString::number (entry.Width_) + 'x' + QString::number (entry.Height_);
			const QVector<QPair<QString, QString>> rows
			{
				IntRow (TrCtx::tr ("Duration"), entry.Duration_),
				IntRow (TrCtx::tr ("Channels"), entry.Channels_),
				{ TrCtx::tr ("Size"), size },
				{ TrCtx::tr ("Bitrate"), entry.Bitrate_ ? TrCtx::tr ("%1 kbps").arg (entry.Bitrate_) : QString {} },
				IntRow (TrCtx::tr ("Framerate"), entry.Framerate_),
				IntRow (TrCtx::tr ("Sampling rate"), entry.SamplingRate_),
				{ TrCtx::tr ("MIME type"), entry.Type_ },
			};

			Nodes nodes;
			nodes.reserve (rows.size ());
			for (const auto& [label, value] : rows)
				if (!value.isEmpty ())
					nodes.push_back (Tags::Li (MakeField (label, value)));

			if (nodes.isEmpty ())
				return {};

			return MakeSubblock (TrCtx::tr ("Technical information"), color, { Tags::Ul (std::move (nodes)) });
		}

		Nodes MakeComments (const QList<MRSSComment>& comments, const TextColor& color)
		{
			if (comments.isEmpty ())
				return {};

			QMap<QString, Nodes> grouped;
			for (const auto& comment : comments)
				grouped [comment.Type_].push_back (Tags::Li ({ comment.Comment_ }));

			Nodes nodes;
			nodes.reserve (grouped.size () * 2);
			for (auto&& [type, typeNodes] : Util::Stlize (grouped))
				nodes += MakeSubblock (type, color, { Tags::Ul (std::move (typeNodes)) });
			return nodes;
		}

		Nodes MakeCopyright (const MRSSEntry& entry)
		{
			if (entry.CopyrightText_.isEmpty () && entry.CopyrightURL_.isEmpty ())
				return {};

			if (entry.CopyrightURL_.isEmpty ())
				return MakeField (TrCtx::tr ("Copyright"), entry.CopyrightURL_);

			const auto& copyrightText = entry.CopyrightText_.isEmpty () ? QStringLiteral ("©") : QString {};
			return MakeField (TrCtx::tr ("Copyright"), { MakeLink (entry.CopyrightURL_, copyrightText) });
		}

		Nodes MakeCredits (const QList<MRSSCredit>& credits, const TextColor& color)
		{
			Nodes nodes;
			nodes.reserve (credits.size ());
			for (const auto& credit : credits)
				if (!credit.Role_.isEmpty ())
					nodes.push_back (Tags::Li ({ credit.Role_ + ": "_qs + credit.Who_ }));

			if (nodes.isEmpty ())
				return {};

			return MakeSubblock (TrCtx::tr ("Credits", "at the end of a video"), color, { Tags::Ul (std::move (nodes)) });
		}
	}

	Nodes MakeMRSSEntry (const MRSSEntry& entry, const TextColor& color)
	{
		return MakeHeader (entry) +
				MakePeerLinks (entry.PeerLinks_, color) +
				MakeDescription (entry) +
				MakeThumbnails (entry.Thumbnails_) +
				MakeExpression (entry.Expression_) +
				MakeScenes (entry.Scenes_, color) +
				MakeStats (entry, color) +
				MakeTechInfo (entry, color) +
				MakeComments (entry.Comments_, color) +
				MakeCopyright (entry) +
				MakeCredits (entry.Credits_, color);
	}
}
