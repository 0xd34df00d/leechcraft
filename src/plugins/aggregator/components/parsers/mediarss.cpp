/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mediarss.h"
#include <optional>
#include <QDomElement>
#include <util/sll/domchildrenrange.h>
#include "item.h"
#include "utils.h"

namespace LC::Aggregator::Parsers::MediaRSS
{
	namespace NS
	{
		const QString MediaRSS = "http://search.yahoo.com/mrss/";
	}

	namespace
	{
		struct Metadata
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

			void AugmentWith (const Metadata& other)
			{
				const auto uniteField = [&] (auto field)
				{
					if (other.*field)
						this->*field = other.*field;
				};

				uniteField (&Metadata::URL_);
				uniteField (&Metadata::Rating_);
				uniteField (&Metadata::RatingScheme_);
				uniteField (&Metadata::Title_);
				uniteField (&Metadata::Description_);
				uniteField (&Metadata::Keywords_);
				uniteField (&Metadata::CopyrightURL_);
				uniteField (&Metadata::CopyrightText_);
				uniteField (&Metadata::RatingAverage_);
				uniteField (&Metadata::RatingCount_);
				uniteField (&Metadata::RatingMin_);
				uniteField (&Metadata::RatingMax_);
				uniteField (&Metadata::Views_);
				uniteField (&Metadata::Favs_);
				uniteField (&Metadata::Tags_);

				Thumbnails_ += other.Thumbnails_;
				Credits_ += other.Credits_;
				Comments_ += other.Comments_;
				PeerLinks_ += other.PeerLinks_;
				Scenes_ += other.Scenes_;
			}

			void SetMrssId (IDType_t mrssId)
			{
				auto setId = [&] (auto& items)
				{
					for (auto& item : items)
						item.MRSSEntryID_ = mrssId;
				};
				setId (Thumbnails_);
				setId (Credits_);
				setId (Comments_);
				setId (PeerLinks_);
				setId (Scenes_);
			}
		};

		Metadata Concat (Metadata first, const Metadata& second)
		{
			first.AugmentWith (second);
			return first;
		}

		std::optional<QDomElement> GetFirstChildNS (const QDomElement& parent, const QString& name)
		{
			for (const auto& elem : Util::DomChildren (parent, name))
				if (elem.namespaceURI () == NS::MediaRSS)
					return elem;
			return {};
		}

		QList<QDomElement> GetAllChildrenNS (const QDomElement& parent, const QString& name)
		{
			QList<QDomElement> result;
			for (const auto& elem : Util::DomChildren (parent, name))
				if (elem.namespaceURI () == NS::MediaRSS)
					result << elem;
			return result;
		}

		std::optional<int> GetIntAttr (const QDomElement& elem, const QString& attrName)
		{
			if (!elem.hasAttribute (attrName))
				return {};

			bool ok = false;
			const auto res = elem.attribute (attrName).toInt (&ok);
			return ok ? res : std::optional<int> {};
		}

		std::optional<QString> GetURL (const QDomElement& element)
		{
			const auto& playerElem = GetFirstChildNS (element, "player");
			if (playerElem && playerElem->hasAttribute ("url"))
				return playerElem->attribute ("url");
			return {};
		}

		QList<MRSSThumbnail> GetThumbnails (const QDomElement& element)
		{
			QList<MRSSThumbnail> result;
			for (const auto& node : GetAllChildrenNS (element, "thumbnail"))
			{
				auto thumb = MRSSThumbnail::CreateForEntry ({});
				thumb.URL_ = node.attribute ("url");
				thumb.Width_ = GetIntAttr (node, "width").value_or (0);
				thumb.Height_ = GetIntAttr (node, "height").value_or (0);
				thumb.Time_ = node.attribute ("time");
				result << thumb;
			}
			return result;
		}

		QList<MRSSCredit> GetCredits (const QDomElement& element)
		{
			QList<MRSSCredit> result;
			for (const auto& node : GetAllChildrenNS (element, "credit"))
			{
				if (!node.hasAttribute ("role"))
					continue;

				auto credit = MRSSCredit::CreateForEntry ({});
				credit.Role_ = node.attribute ("role");
				credit.Who_ = node.text ();
				result << credit;
			}
			return result;
		}

		QList<MRSSComment> GetComments (const QDomElement& element)
		{
			QList<MRSSComment> result;
			const auto getSpecificType = [&] (const QString& nodeName, const QString& humanLabel)
			{
				if (const auto& comments = GetFirstChildNS (element, nodeName + "s"))
					for (const auto& node : GetAllChildrenNS (element, nodeName))
					{
						auto comment = MRSSComment::CreateForEntry ({});
						comment.Type_ = humanLabel;
						comment.Comment_ = node.text ();
						result << comment;
					}
			};
			getSpecificType ("comment", QObject::tr ("Comments"));
			getSpecificType ("response", QObject::tr ("Responses"));
			getSpecificType ("backLinks", QObject::tr ("Backlinks"));
			return result;
		}

		QList<MRSSPeerLink> GetPeerLinks (const QDomElement& element)
		{
			QList<MRSSPeerLink> result;
			for (const auto& linkNode : GetAllChildrenNS (element, "peerLink"))
			{
				auto link = MRSSPeerLink::CreateForEntry ({});
				link.Link_ = linkNode.attribute ("href");
				link.Type_ = linkNode.attribute ("type");
				result << link;
			}
			return result;
		}

		QList<MRSSScene> GetScenes (const QDomElement& element)
		{
			QList<MRSSScene> result;
			if (const auto& scenes = GetFirstChildNS (element, "scenes"))
				for (const auto& sceneNode : GetAllChildrenNS (*scenes, "scene"))
				{
					auto scene = MRSSScene::CreateForEntry (IDNotFound);
					scene.Title_ = sceneNode.firstChildElement ("sceneTitle").text ();
					scene.Description_ = sceneNode.firstChildElement ("sceneDescription").text ();
					scene.StartTime_ = sceneNode.firstChildElement ("sceneStartTime").text ();
					scene.EndTime_ = sceneNode.firstChildElement ("sceneEndTime").text ();
					result << scene;
				}
			return result;
		}

		Metadata ParseMetadata (const QDomElement& element)
		{
			Metadata result;

			result.URL_ = GetURL (element);
			result.Title_ = GetFirstNodeText (element, NS::MediaRSS, "title");
			result.Description_ = GetFirstNodeText (element, NS::MediaRSS, "description");
			result.Keywords_ = GetFirstNodeText (element, NS::MediaRSS, "keywords");

			if (const auto& rating = GetFirstChildNS (element, "rating"))
			{
				result.Rating_ = rating->text ();
				result.RatingScheme_ = rating->attribute ("scheme", "urn:simple");
			}

			if (const auto& copyright = GetFirstChildNS (element, "copyright"))
			{
				result.CopyrightText_ = copyright->text ();
				if (copyright->hasAttribute ("url"))
					result.CopyrightURL_ = copyright->attribute ("url");
			}

			if (const auto& community = GetFirstChildNS (element, "community"))
			{
				if (const auto& rating = GetFirstChildNS (*community, "starRating"))
				{
					result.RatingAverage_ = GetIntAttr (*rating, "average");
					result.RatingCount_ = GetIntAttr (*rating, "count");
					result.RatingMin_ = GetIntAttr (*rating, "min");
					result.RatingMax_ = GetIntAttr (*rating, "max");
				}

				if (const auto& stats = GetFirstChildNS (*community, "statistics"))
				{
					result.Views_ = GetIntAttr (*stats, "views");
					result.Favs_ = GetIntAttr (*stats, "favorites");
				}

				if (const auto& tags = GetFirstChildNS (*community, "tags"))
					result.Tags_ = tags->text ();
			}

			result.Thumbnails_ = GetThumbnails (element);
			result.Credits_ = GetCredits (element);
			result.Comments_ = GetComments (element);
			result.PeerLinks_ = GetPeerLinks (element);
			result.Scenes_ = GetScenes (element);

			return result;
		}

		void FillEntry (MRSSEntry& entry, const QDomElement& elem)
		{
			if (elem.hasAttribute ("url"))
				entry.URL_ = elem.attribute ("url");
			else
				entry.URL_ = GetURL (elem).value_or (QString {});

			entry.Size_ = elem.attribute ("fileSize").toInt ();
			entry.Type_ = elem.attribute ("type");
			entry.Medium_ = elem.attribute ("medium");
			entry.IsDefault_ = elem.attribute ("isDefault") == "true";
			entry.Expression_ = elem.attribute ("expression", "full");
			entry.Bitrate_ = elem.attribute ("bitrate").toInt ();
			entry.Framerate_ = elem.attribute ("framerate").toDouble ();
			entry.SamplingRate_ = elem.attribute ("samplingrate").toDouble ();
			entry.Channels_ = elem.attribute ("channels").toInt ();
			entry.Duration_ = elem.attribute ("duration").toInt ();
			entry.Width_ = elem.attribute ("width").toInt ();
			entry.Height_ = elem.attribute ("height").toInt ();
			entry.Lang_ = elem.attribute ("lang");
		}

		void FillWithMetadata (MRSSEntry& entry, const Metadata& md)
		{
			entry.Rating_ = md.Rating_.value_or (QString {});
			entry.RatingScheme_ = md.RatingScheme_.value_or (QString {});
			entry.Title_ = md.Title_.value_or (QString {});
			entry.Description_ = md.Description_.value_or (QString {});
			entry.Keywords_ = md.Keywords_.value_or (QString {});
			entry.CopyrightURL_ = md.CopyrightURL_.value_or (QString {});
			entry.CopyrightText_ = md.CopyrightText_.value_or (QString {});
			entry.RatingAverage_ = md.RatingAverage_.value_or (0);
			entry.RatingCount_ = md.RatingCount_.value_or (0);
			entry.RatingMin_ = md.RatingMin_.value_or (0);
			entry.RatingMax_ = md.RatingMax_.value_or (0);
			entry.Views_ = md.Views_.value_or (0);
			entry.Favs_ = md.Favs_.value_or (0);
			entry.Tags_ = md.Tags_.value_or (QString {});
			entry.Thumbnails_ = md.Thumbnails_;
			entry.Credits_ = md.Credits_;
			entry.Comments_ = md.Comments_;
			entry.PeerLinks_ = md.PeerLinks_;
			entry.Scenes_ = md.Scenes_;
		}

		QList<MRSSEntry> ParseChildren (const QDomElement& parent, const Metadata& groupMetadata, IDType_t itemId)
		{
			const auto& entries = parent.elementsByTagNameNS (NS::MediaRSS, "content");
			const auto size = entries.size ();

			QList<MRSSEntry> result;
			result.reserve (size);
			for (int i = 0; i < size; ++i)
			{
				const auto& elem = entries.at (i).toElement ();

				auto entry = MRSSEntry::CreateForItem (itemId);

				auto elemMetadata = Concat (groupMetadata, ParseMetadata (elem));
				elemMetadata.SetMrssId (entry.MRSSEntryID_);

				FillEntry (entry, elem);
				FillWithMetadata (entry, elemMetadata);
			}

			return result;
		}
	}

	QList<MRSSEntry> Parse (const QDomElement& item, IDType_t itemId)
	{
		QList<MRSSEntry> result;

		const auto& rootMetadata = ParseMetadata (item);

		auto groups = item.elementsByTagNameNS (NS::MediaRSS, "group");
		for (int i = 0; i < groups.size (); ++i)
		{
			const auto& group = groups.at (i).toElement ();

			const auto& groupMetadata = Concat (rootMetadata, ParseMetadata (group));

			result += ParseChildren (group, groupMetadata, itemId);
		}

		result += ParseChildren (item, rootMetadata, itemId);

		return result;
	}
}
