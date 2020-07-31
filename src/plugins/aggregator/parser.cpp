/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "parser.h"
#include <optional>
#include <QDomElement>
#include <QStringList>
#include <QObject>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/domchildrenrange.h>

uint qHash (const QDomNode& node)
{
	if (node.lineNumber () == -1 ||
			node.columnNumber () == -1)
	{
		qWarning () << Q_FUNC_INFO
			<< "node is unhasheable";
		return -1;
	}
	return (node.lineNumber () << 24) + node.columnNumber ();
}

namespace LC
{
namespace Aggregator
{
	const QString Parser::DC_ = "http://purl.org/dc/elements/1.1/";
	const QString Parser::WFW_ = "http://wellformedweb.org/CommentAPI/";
	const QString Parser::Atom_ = "http://www.w3.org/2005/Atom";
	const QString Parser::RDF_ = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
	const QString Parser::Slash_ = "http://purl.org/rss/1.0/modules/slash/";
	const QString Parser::Enc_ = "http://purl.oclc.org/net/rss_2.0/enc#";
	const QString Parser::ITunes_ = "http://www.itunes.com/dtds/podcast-1.0.dtd";
	const QString Parser::GeoRSSSimple_ = "http://www.georss.org/georss";
	const QString Parser::GeoRSSW3_ = "http://www.w3.org/2003/01/geo/wgs84_pos#";
	const QString Parser::MediaRSS_ = "http://search.yahoo.com/mrss/";
	const QString Parser::Content_ = "http://purl.org/rss/1.0/modules/content/";

	channels_container_t Parser::ParseFeed (const QDomDocument& recent, const IDType_t& feedId) const
	{
		channels_container_t newes = Parse (recent, feedId);
		for (const auto& newChannel : newes)
		{
			if (newChannel->Link_.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
					<< "detected empty link for"
					<< newChannel->Title_;
				newChannel->Link_ = "about:blank";
			}
			for (const auto& item : newChannel->Items_)
				item->Title_ = item->Title_.trimmed ().simplified ();
		}
		return newes;
	}

	namespace
	{
		QList<QString> ToList (const QDomNodeList& dumbList)
		{
			QList<QString> nodes;
			for (int i = 0, size = dumbList.size (); i < size; ++i)
				nodes << dumbList.at (i).toElement ().text ();
			return nodes;
		}
	};

	QString Parser::GetDescription (const QDomElement& parent) const
	{
		auto texts = ToList (parent.elementsByTagNameNS (Content_, "encoded")) +
				ToList (parent.elementsByTagNameNS (ITunes_, "summary"));

		if (texts.isEmpty ())
			return {};

		return *std::max_element (texts.begin (), texts.end (), Util::ComparingBy (&QString::size));
	}

	void Parser::GetDescription (const QDomElement& parent,
			QString& cand) const
	{
		QString extContent = GetDescription (parent);
		if (extContent.size () > cand.size ())
			cand = extContent;
	}

	QString Parser::GetLink (const QDomElement& parent) const
	{
		for (const auto& link : Util::DomChildren (parent, "link"))
		{
			if (link.hasAttribute ("rel") && link.attribute ("rel") != "alternate")
				continue;

			return link.hasAttribute ("href") ?
					link.attribute ("href") :
					link.text ();
		}
		return {};
	}

	QString Parser::GetAuthor (const QDomElement& parent) const
	{
		QString result;
		QDomNodeList nodes = parent.elementsByTagNameNS (ITunes_,
				"author");
		if (nodes.size ())
		{
			result = nodes.at (0).toElement ().text ();
			return result;
		}

		nodes = parent.elementsByTagNameNS (DC_,
				"creator");
		if (nodes.size ())
		{
			result = nodes.at (0).toElement ().text ();
			return result;
		}

		nodes = parent.elementsByTagName ("author");
		if (nodes.size ())
		{
			result = nodes.at (0).toElement ().text ();
			return result;
		}

		return result;
	}

	QString Parser::GetCommentsRSS (const QDomElement& parent) const
	{
		QString result;
		QDomNodeList nodes = parent.elementsByTagNameNS (WFW_,
				"commentRss");
		if (nodes.size ())
			result = nodes.at (0).toElement ().text ();
		return result;
	}

	QString Parser::GetCommentsLink (const QDomElement& parent) const
	{
		QString result;
		QDomNodeList nodes = parent.elementsByTagNameNS ("", "comments");
		if (nodes.size ())
			result = nodes.at (0).toElement ().text ();
		return result;
	}

	int Parser::GetNumComments (const QDomElement& parent) const
	{
		int result = -1;
		QDomNodeList nodes = parent.elementsByTagNameNS (Slash_,
				"comments");
		if (nodes.size ())
			result = nodes.at (0).toElement ().text ().toInt ();
		return result;
	}

	QDateTime Parser::GetDCDateTime (const QDomElement& parent) const
	{
		QDomNodeList dates = parent.elementsByTagNameNS (DC_, "date");
		if (!dates.size ())
			return QDateTime ();
		return FromRFC3339 (dates.at (0).toElement ().text ());
	}

	QStringList Parser::GetAllCategories (const QDomElement& parent) const
	{
		return GetDCCategories (parent) +
			GetPlainCategories (parent) +
			GetITunesCategories (parent);
	}

	QStringList Parser::GetDCCategories (const QDomElement& parent) const
	{
		QStringList result;

		QDomNodeList nodes =
			parent.elementsByTagNameNS (DC_,
					"subject");
		for (int i = 0; i < nodes.size (); ++i)
			result += nodes.at (i).toElement ().text ();

		result.removeAll ("");

		return result;
	}

	QStringList Parser::GetITunesCategories (const QDomElement& parent) const
	{
		QStringList result;

		QDomNodeList nodes =
			parent.elementsByTagNameNS (ITunes_,
					"keywords");
		for (int i = 0; i < nodes.size (); ++i)
			/*: This is the template for the category created of
				* iTunes podcast keywords.
				*/
			result += QString (QObject::tr ("Podcast %1")
					.arg (nodes.at (i).toElement ().text ()));

		result.removeAll ("");
		return result;
	}

	QStringList Parser::GetPlainCategories (const QDomElement& parent) const
	{
		QStringList result;

		QDomNodeList nodes =
			parent.elementsByTagName ("category");
		for (int i = 0; i < nodes.size (); ++i)
			result += nodes.at (i).toElement ().text ();

		result.removeAll ("");

		return result;
	}

	QList<Enclosure> Parser::GetEncEnclosures (const QDomElement& parent,
			const IDType_t& itemId) const
	{
		QList<Enclosure> result;

		QDomNodeList nodes = parent.elementsByTagNameNS (Enc_, "enclosure");

		for (int i = 0; i < nodes.size (); ++i)
		{
			QDomElement link = nodes.at (i).toElement ();

			auto e = Enclosure::CreateForItem (itemId);
			e.URL_ = link.attributeNS (RDF_, "resource");
			e.Type_ = link.attributeNS (Enc_, "type");
			e.Length_ = link.attributeNS (Enc_, "length", "-1").toLongLong ();
			e.Lang_ = "";

			result << e;
		}

		return result;
	}

	QPair<double, double> Parser::GetGeoPoint (const QDomElement& parent) const
	{
		QPair<double, double> result = qMakePair<double, double> (0, 0);

		QDomNodeList lats = parent.elementsByTagNameNS (GeoRSSW3_, "lat");
		QDomNodeList longs = parent.elementsByTagNameNS (GeoRSSW3_, "long");
		if (lats.size () && longs.size ())
		{
			result.first = lats.at (0).toElement ().text ().toDouble ();
			result.second = longs.at (0).toElement ().text ().toDouble ();
		}
		else
		{
			QDomNodeList points = parent.elementsByTagNameNS (GeoRSSSimple_, "point");
			if (points.size ())
			{
				const auto& text = points.at (0).toElement ().text ();
				const auto& splitted = text.splitRef (' ');
				if (splitted.size () == 2)
				{
					result.first = splitted.at (0).toDouble ();
					result.second = splitted.at (1).toDouble ();
				}
			}
		}

		return result;
	}

	namespace
	{
		QList<QDomNode> GetDirectChildrenNS (const QDomElement& elem,
				const QString& ns, const QString& name)
		{
			QList<QDomNode> result;
			QDomNodeList unf = elem.elementsByTagNameNS (ns, name);
			for (int i = 0, size = unf.size (); i < size; ++i)
				if (unf.at (i).parentNode () == elem)
					result << unf.at (i);
			return result;
		}
	}

	class MRSSParser
	{
		struct ArbitraryLocatedData
		{
			std::optional<QString> URL_;
			std::optional<QString> Rating_;
			std::optional<QString> RatingScheme_;
			std::optional<QString> Title_;
			std::optional<QString> Description_;
			std::optional<QString> Keywords_;
			std::optional<QString> CopyrightURL_;
			std::optional<QString> CopyrightText_;
			std::optional<int> RatingAverage_;
			std::optional<int> RatingCount_;
			std::optional<int> RatingMin_;
			std::optional<int> RatingMax_;
			std::optional<int> Views_;
			std::optional<int> Favs_;
			std::optional<QString> Tags_;
			QList<MRSSThumbnail> Thumbnails_;
			QList<MRSSCredit> Credits_;
			QList<MRSSComment> Comments_;
			QList<MRSSPeerLink> PeerLinks_;
			QList<MRSSScene> Scenes_;

			/**  Updates *this's fields according to the
				* child. Some kind of merge.
				*/
			ArbitraryLocatedData& operator+= (const ArbitraryLocatedData& child)
			{
				if (child.URL_)
					URL_ = child.URL_;
				if (child.Rating_)
					Rating_ = child.Rating_;
				if (child.RatingScheme_)
					RatingScheme_ = child.RatingScheme_;
				if (child.Title_)
					Title_ = child.Title_;
				if (child.Description_)
					Description_ = child.Description_;
				if (child.Keywords_)
					Keywords_ = child.Keywords_;
				if (child.CopyrightURL_)
					CopyrightURL_ = child.CopyrightURL_;
				if (child.CopyrightText_)
					CopyrightText_ = child.CopyrightText_;
				if (child.RatingAverage_)
					RatingAverage_ = child.RatingAverage_;
				if (child.RatingCount_)
					RatingCount_ = child.RatingCount_;
				if (child.RatingMin_)
					RatingMin_ = child.RatingMin_;
				if (child.RatingMax_)
					RatingMax_ = child.RatingMax_;
				if (child.Views_)
					Views_ = child.Views_;
				if (child.Favs_)
					Favs_ = child.Favs_;
				if (child.Tags_)
					Tags_ = child.Tags_;

				Thumbnails_ += child.Thumbnails_;
				Credits_ += child.Credits_;
				Comments_ += child.Comments_;
				PeerLinks_ += child.PeerLinks_;
				Scenes_ += child.Scenes_;
				return *this;
			}
		};

		QHash<QDomNode, ArbitraryLocatedData> Cache_;

		IDType_t ItemID_;
	public:
		MRSSParser (const IDType_t& itemId) : ItemID_ (itemId) {}

		QList<MRSSEntry> operator() (const QDomElement& item)
		{
			QList<MRSSEntry> result;

			QDomNodeList groups = item.elementsByTagNameNS (Parser::MediaRSS_,
					"group");
			for (int i = 0; i < groups.size (); ++i)
				result += CollectChildren (groups.at (i).toElement ());

			result += CollectChildren (item);

			return result;
		}
	private:
		QList<MRSSEntry> CollectChildren (const QDomElement& holder)
		{
			QList<MRSSEntry> result;
			QDomNodeList entries = holder.elementsByTagNameNS (Parser::MediaRSS_,
					"content");
			for (int i = 0; i < entries.size (); ++i)
			{
				auto entry = MRSSEntry::CreateForItem (ItemID_);

				QDomElement en = entries.at (i).toElement ();
				ArbitraryLocatedData d = GetArbitraryLocatedDataFor (en, entry.MRSSEntryID_);

				if (en.hasAttribute ("url"))
					entry.URL_ = en.attribute ("url");
				else
				{
					QDomNodeList players = en.elementsByTagNameNS (Parser::MediaRSS_,
							"player");
					if (!players.size ())
						qWarning () << Q_FUNC_INFO
							<< "bad feed with no players and urls";
					entry.URL_ = players.at (0).toElement ().attribute ("url");
				}

				entry.Size_ = en.attribute ("fileSize").toInt ();
				entry.Type_ = en.attribute ("type");
				entry.Medium_ = en.attribute ("medium");
				entry.IsDefault_ = (en.attribute ("isDefault") == "true");
				entry.Expression_ = en.attribute ("expression");
				if (entry.Expression_.isEmpty ())
					entry.Expression_ = "full";
				entry.Bitrate_ = en.attribute ("bitrate").toInt ();
				entry.Framerate_ = en.attribute ("framerate").toDouble ();
				entry.SamplingRate_ = en.attribute ("samplingrate").toDouble ();
				entry.Channels_ = en.attribute ("channels").toInt ();
				entry.Duration_ = en.attribute ("duration").toInt ();
				entry.Width_ = en.attribute ("width").toInt ();
				entry.Height_ = en.attribute ("height").toInt ();
				entry.Lang_ = en.attribute ("lang");

				entry.Rating_ = d.Rating_.value_or (QString ());
				entry.RatingScheme_ = d.RatingScheme_.value_or (QString ());
				entry.Title_ = d.Title_.value_or (QString ());
				entry.Description_ = d.Description_.value_or (QString ());
				entry.Keywords_ = d.Keywords_.value_or (QString ());
				entry.CopyrightURL_ = d.CopyrightURL_.value_or (QString ());
				entry.CopyrightText_ = d.CopyrightText_.value_or (QString ());
				entry.RatingAverage_ = d.RatingAverage_.value_or (0);
				entry.RatingCount_ = d.RatingCount_.value_or (0);
				entry.RatingMin_ = d.RatingMin_.value_or (0);
				entry.RatingMax_ = d.RatingMax_.value_or (0);
				entry.Views_ = d.Views_.value_or (0);
				entry.Favs_ = d.Favs_.value_or (0);
				entry.Tags_ = d.Tags_.value_or (QString ());
				entry.Thumbnails_ = d.Thumbnails_;
				entry.Credits_ = d.Credits_;
				entry.Comments_ = d.Comments_;
				entry.PeerLinks_ = d.PeerLinks_;
				entry.Scenes_ = d.Scenes_;

				result << entry;
			}
			return result;
		}

		ArbitraryLocatedData GetArbitraryLocatedDataFor (const QDomElement& holder,
				const IDType_t& mrssId)
		{
			ArbitraryLocatedData result;

			QList<QDomElement> parents;
			QDomElement parent = holder;
			while (!parent.isNull ())
			{
				parents.prepend (parent);
				parent = parent.parentNode ().toElement ();
			}
			for (const auto& p : parents)
				result += CollectArbitraryLocatedData (p, mrssId);
			return result;
		}

		std::optional<QString> GetURL (const QDomElement& element)
		{
			QList<QDomNode> elems = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"player");
			if (!elems.size ())
				return {};

			return elems.at (0).toElement ().attribute ("url");
		}

		std::optional<QString> GetTitle (const QDomElement& element)
		{
			QList<QDomNode> elems = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"title");
			if (!elems.size ())
				return {};

			QDomElement telem = elems.at (0).toElement ();
			return Parser::UnescapeHTML (telem.text ());
		}

		std::optional<QString> GetDescription (const QDomElement& element)
		{
			QList<QDomNode> elems = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"description");
			if (!elems.size ())
				return {};

			QDomElement telem = elems.at (0).toElement ();
			return Parser::UnescapeHTML (telem.text ());
		}

		std::optional<QString> GetKeywords (const QDomElement& element)
		{
			QList<QDomNode> elems = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"keywords");
			if (!elems.size ())
				return {};

			QDomElement telem = elems.at (0).toElement ();
			return telem.text ();
		}

		std::optional<int> GetInt (const QDomElement& elem, const QString& attrname)
		{
			if (elem.hasAttribute (attrname))
			{
				bool ok = false;
				int result = elem.attribute (attrname).toInt (&ok);
				if (ok)
					return result;
			}
			return {};
		}

		QList<MRSSThumbnail> GetThumbnails (const QDomElement& element,
				const IDType_t& mrssId)
		{
			QList<MRSSThumbnail> result;
			QList<QDomNode> thumbs = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"thumbnail");;
			for (int i = 0; i < thumbs.size (); ++i)
			{
				QDomElement thumbNode = thumbs.at (i).toElement ();

				auto thumb = MRSSThumbnail::CreateForEntry (mrssId);
				thumb.URL_ = thumbNode.attribute ("url");
				thumb.Width_ = GetInt (thumbNode, "width").value_or (0);
				thumb.Height_ = GetInt (thumbNode, "height").value_or (0);
				thumb.Time_ = thumbNode.attribute ("time");
				result << thumb;
			}
			return result;
		}

		QList<MRSSCredit> GetCredits (const QDomElement& element,
				const IDType_t& mrssId)
		{
			QList<MRSSCredit> result;
			QList<QDomNode> credits = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"credit");
			for (int i = 0; i < credits.size (); ++i)
			{
				QDomElement creditNode = credits.at (i).toElement ();
				if (!creditNode.hasAttribute ("role"))
					continue;
				auto credit = MRSSCredit::CreateForEntry (mrssId);
				credit.Role_ = creditNode.attribute ("role");
				credit.Who_ = creditNode.text ();
				result << credit;
			}
			return result;
		}

		QList<MRSSComment> GetComments (const QDomElement& element,
				const IDType_t& mrssId)
		{
			QList<MRSSComment> result;
			QList<QDomNode> commParents = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"comments");
			if (commParents.size ())
			{
				QDomNodeList comments = commParents.at (0).toElement ()
					.elementsByTagNameNS (Parser::MediaRSS_,
						"comment");
				for (int i = 0; i < comments.size (); ++i)
				{
					auto comment = MRSSComment::CreateForEntry (mrssId);
					comment.Type_ = QObject::tr ("Comments");
					comment.Comment_ = comments.at (i).toElement ().text ();
					result << comment;
				}
			}

			QList<QDomNode> respParents = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"responses");
			if (respParents.size ())
			{
				QDomNodeList responses = respParents.at (0).toElement ()
					.elementsByTagNameNS (Parser::MediaRSS_,
						"response");
				for (int i = 0; i < responses.size (); ++i)
				{
					auto comment = MRSSComment::CreateForEntry (mrssId);
					comment.Type_ = QObject::tr ("Responses");
					comment.Comment_ = responses.at (i).toElement ().text ();
					result << comment;
				}
			}

			QList<QDomNode> backParents = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"backLinks");
			if (backParents.size ())
			{
				QDomNodeList backlinks = backParents.at (0).toElement ()
					.elementsByTagNameNS (Parser::MediaRSS_,
						"backLink");
				for (int i = 0; i < backlinks.size (); ++i)
				{
					auto comment = MRSSComment::CreateForEntry (mrssId);
					comment.Type_ = QObject::tr ("Backlinks");
					comment.Comment_ = backlinks.at (i).toElement ().text ();
					result << comment;
				}
			}
			return result;
		}

		QList<MRSSPeerLink> GetPeerLinks (const QDomElement& element,
				const IDType_t& mrssId)
		{
			QList<MRSSPeerLink> result;
			QList<QDomNode> links = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"peerLink");
			for (int i = 0; i < links.size (); ++i)
			{
				QDomElement linkNode = links.at (i).toElement ();
				auto pl = MRSSPeerLink::CreateForEntry (mrssId);
				pl.Link_ = linkNode.attribute ("href");
				pl.Type_ = linkNode.attribute ("type");
				result << pl;
			}
			return result;
		}

		QList<MRSSScene> GetScenes (const QDomElement& element,
				const IDType_t& mrssId)
		{
			QList<MRSSScene> result;
			QList<QDomNode> scenesNode = GetDirectChildrenNS (element, Parser::MediaRSS_,
					"scenes");
			if (scenesNode.size ())
			{
				QDomNodeList scenesNodes = scenesNode.at (0).toElement ()
					.elementsByTagNameNS (Parser::MediaRSS_, "scene");
				for (int i = 0; i < scenesNodes.size (); ++i)
				{
					QDomElement sceneNode = scenesNodes.at (i).toElement ();
					auto scene = MRSSScene::CreateForEntry (mrssId);
					scene.Title_ = sceneNode.firstChildElement ("sceneTitle").text ();
					scene.Description_ = sceneNode.firstChildElement ("sceneDescription").text ();
					scene.StartTime_ = sceneNode.firstChildElement ("sceneStartTime").text ();
					scene.EndTime_ = sceneNode.firstChildElement ("sceneEndTime").text ();
					result << scene;
				}
			}
			return result;
		}

		ArbitraryLocatedData CollectArbitraryLocatedData (const QDomElement& element,
				const IDType_t& mrssId)
		{
			if (Cache_.contains (element))
				return Cache_ [element];

			std::optional<QString> rating;
			std::optional<QString> rscheme;
			{
				QList<QDomNode> elems = GetDirectChildrenNS (element, Parser::MediaRSS_,
						"rating");
				if (elems.size ())
				{
					QDomElement relem = elems.at (0).toElement ();
					rating = relem.text ();
					if (relem.hasAttribute ("scheme"))
						rscheme = relem.attribute ("scheme");
					else
						rscheme = "urn:simple";
				}
			}

			std::optional<QString> curl;
			std::optional<QString> ctext;
			{
				QList<QDomNode> elems = GetDirectChildrenNS (element, Parser::MediaRSS_,
						"copyright");
				if (elems.size ())
				{
					QDomElement celem = elems.at (0).toElement ();
					ctext = celem.text ();
					if (celem.hasAttribute ("url"))
						curl = celem.attribute ("url");
				}
			}
			std::optional<int> raverage {};
			std::optional<int> rcount {};
			std::optional<int> rmin {};
			std::optional<int> rmax {};
			std::optional<int> views {};
			std::optional<int> favs {};
			std::optional<QString> tags;
			{
				QList<QDomNode> comms = GetDirectChildrenNS (element, Parser::MediaRSS_,
						"community");
				if (comms.size ())
				{
					QDomElement comm = comms.at (0).toElement ();
					QDomNodeList stars = comm.elementsByTagNameNS (Parser::MediaRSS_,
							"starRating");
					if (stars.size ())
					{
						QDomElement rating = stars.at (0).toElement ();
						raverage = GetInt (rating, "average");
						rcount = GetInt (rating, "count");
						rmin = GetInt (rating, "min");
						rmax = GetInt (rating, "max");
					}

					QDomNodeList stats = comm.elementsByTagNameNS (Parser::MediaRSS_,
							"statistics");
					if (stats.size ())
					{
						QDomElement stat = stats.at (0).toElement ();
						views = GetInt (stat, "views");
						favs = GetInt (stat, "favorites");
					}

					QDomNodeList tagsNode = comm.elementsByTagNameNS (Parser::MediaRSS_,
							"tags");
					if (tagsNode.size ())
					{
						QDomElement tag = tagsNode.at (0).toElement ();
						tags = tag.text ();
					}
				}
			}

			ArbitraryLocatedData result =
			{
				GetURL (element),
				rating,
				rscheme,
				GetTitle (element),
				GetDescription (element),
				GetKeywords (element),
				curl,
				ctext,
				raverage,
				rcount,
				rmin,
				rmax,
				views,
				favs,
				tags,
				GetThumbnails (element, mrssId),
				GetCredits (element, mrssId),
				GetComments (element, mrssId),
				GetPeerLinks (element, mrssId),
				GetScenes (element, mrssId)
			};

			Cache_ [element] = result;
			return result;
		}
	};

	QList<MRSSEntry> Parser::GetMediaRSS (const QDomElement& item,
			const IDType_t& itemId) const
	{
		return MRSSParser (itemId) (item);
	}

	QDateTime Parser::FromRFC3339 (const QString& t) const
	{
		if (t.size () < 19)
			return QDateTime ();

		auto result = QDateTime::fromString (t.left (19).toUpper (), "yyyy-MM-ddTHH:mm:ss");
		QRegExp fractionalSeconds ("(\\.)(\\d+)");
		if (fractionalSeconds.indexIn (t) > -1)
		{
			bool ok;
			int fractional = fractionalSeconds.cap (2).toInt (&ok);
			if (ok)
			{
				if (fractional < 100)
					fractional *= 10;
				if (fractional < 10)
					fractional *= 100;
				result = result.addMSecs (fractional);
			}
		}

		QRegExp timeZone ("(\\+|\\-)(\\d\\d)(:)(\\d\\d)$");
		if (timeZone.indexIn (t) > -1)
		{
			short int multiplier = -1;
			if (timeZone.cap (1) == "-")
				multiplier = 1;

			const int hoursShift = timeZone.cap (2).toInt ();
			const int minutesShift = timeZone.cap (4).toInt ();
			result = result.addSecs (hoursShift * 3600 * multiplier + minutesShift * 60 * multiplier);
		}

		result.setTimeSpec (Qt::UTC);
		return result.toLocalTime ();
	}

	// Via
	// http://www.theukwebdesigncompany.com/articles/entity-escape-characters.php
	QString Parser::UnescapeHTML (const QString& escaped)
	{
		QString result = escaped;
		result.replace ("&euro;", "€");
		result.replace ("&quot;", "\"");
		result.replace ("&amp;", "&");
		result.replace ("&nbsp;", " ");
		result.replace ("&lt;", "<");
		result.replace ("&gt;", ">");
		result.replace ("&#8217;", "'");
		result.replace ("&#8230;", "...");
		return result;
	}
}
}
