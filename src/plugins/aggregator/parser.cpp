/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "parser.h"
#include <QDomElement>
#include <QStringList>
#include <QObject>
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
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

			Parser::~Parser ()
			{
			}
			
			channels_container_t Parser::Parse (const channels_container_t& channels,
					channels_container_t& modified,
					const QDomDocument& recent) const
			{
				channels_container_t newes = Parse (recent),
					result;
				for (size_t i = 0; i < newes.size (); ++i)
				{
					Channel_ptr newChannel = newes [i];
					int position = -1;
					for (size_t j = 0; j < channels.size (); ++j)
						if (channels [j]->Title_ == newChannel->Title_ &&
								channels [j]->Link_ == newChannel->Link_)
						{
							position = j;
							break;
						}
			
					if (position == -1)
						result.push_back (newChannel);
					else if (!channels [position]->Items_.size ())
					{
						Channel_ptr pointer = channels [position];
						pointer->Items_ = newChannel->Items_;
						result.push_back (pointer);
					}
					else
					{
						Channel_ptr oldChannel = channels [position];
						Channel_ptr toInsert (new Channel ());
						Channel_ptr modifiedContainer (new Channel ());
						toInsert->Equalify (*oldChannel);
						toInsert->LastBuild_ = newChannel->LastBuild_;
						modifiedContainer->Equalify (*oldChannel);
			
						for (size_t j = 0; j < newChannel->Items_.size (); ++j)
						{
							items_container_t::const_iterator place =
								std::find_if (oldChannel->Items_.begin (),
										oldChannel->Items_.end (),
										ItemComparator (newChannel->Items_ [j]));
			
							if (place == oldChannel->Items_.end ())
								toInsert->Items_.push_back (newChannel->Items_ [j]);
							else
								modifiedContainer->Items_.push_back (newChannel->Items_ [j]);
						}
						result.push_back (toInsert);
						modified.push_back (modifiedContainer);
					}
				}
				return result;
			}
			
			QString Parser::GetLink (const QDomElement& parent) const
			{
				QString result;
				QDomElement link = parent.firstChildElement ("link");
				while (!link.isNull ())
				{
					if (!link.hasAttribute ("rel") || link.attribute ("rel") == "alternate")
					{
						if (!link.hasAttribute ("href"))
							result = link.text ();
						else
							result = link.attribute ("href");
						break;
					}
					link = link.nextSiblingElement ("link");
				}
				return result;
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
			
			QList<Enclosure> Parser::GetEncEnclosures (const QDomElement& parent) const
			{
				QList<Enclosure> result;
			
				QDomNodeList nodes = parent.elementsByTagNameNS (Enc_, "enclosure");
			
				for (int i = 0; i < nodes.size (); ++i)
				{
					QDomElement link = nodes.at (i).toElement ();
			
					Enclosure e =
					{
						link.attributeNS (RDF_, "resource"),
						link.attributeNS (Enc_, "type"),
						link.attributeNS (Enc_, "length", "-1").toLongLong (),
						""
					};
			
					result << e;
				}
			
				return result;
			}
			
			// Via
			// http://www.theukwebdesigncompany.com/articles/entity-escape-characters.php
			QString Parser::UnescapeHTML (const QString& escaped) const
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
			
			QDateTime Parser::FromRFC3339 (const QString& t) const
			{
				int hoursShift = 0, minutesShift = 0;
				if (t.size () < 19)
					return QDateTime ();
				QDateTime result = QDateTime::fromString (t.left (19).toUpper (), "yyyy-MM-ddTHH:mm:ss");
				QRegExp fractionalSeconds ("(\\.)(\\d+)");
				if (fractionalSeconds.indexIn (t) > -1)
				{
					bool ok;
					int fractional = fractionalSeconds.cap (2).toInt (&ok);
					if (ok)
					{
						if (fractional < 100)
							fractional *= 10;
						if (fractional <10) 
							fractional *= 100;
						result.addMSecs (fractional);
					}
				}
				QRegExp timeZone ("(\\+|\\-)(\\d\\d)(:)(\\d\\d)$");
				if (timeZone.indexIn (t) > -1)
				{
					short int multiplier = -1;
					if (timeZone.cap (1) == "-")
						multiplier = 1;
					hoursShift = timeZone.cap (2).toInt ();
					minutesShift = timeZone.cap (4).toInt ();
					result = result.addSecs (hoursShift * 3600 * multiplier + minutesShift * 60 * multiplier);
				}
				result.setTimeSpec (Qt::UTC);
				return result.toLocalTime ();
			}
		};
	};
};

