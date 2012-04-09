/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "icalparser.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/adapted.hpp>
#include <QtDebug>

namespace LeechCraft
{
namespace Otlozhu
{
	namespace
	{
		typedef boost::variant<std::string> FieldVal_t;
		struct Field
		{
			std::string Name_;
			FieldVal_t Val_;
		};
		typedef std::vector<Field> Fields_t;

		struct Item
		{
			std::string Name_;
			Fields_t Fields_;
		};
		typedef std::vector<Item> Items_t;

		struct ICal
		{
			Fields_t Fields_;
			Items_t Items_;
		};
	}
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Otlozhu::Field,
		(std::string, Name_)
		(LeechCraft::Otlozhu::FieldVal_t, Val_));

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Otlozhu::Item,
		(std::string, Name_)
		(LeechCraft::Otlozhu::Fields_t, Fields_));

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Otlozhu::ICal,
		(LeechCraft::Otlozhu::Fields_t, Fields_)
		(LeechCraft::Otlozhu::Items_t, Items_));

namespace LeechCraft
{
namespace Otlozhu
{
	namespace
	{
		namespace ascii = boost::spirit::ascii;
		namespace qi = boost::spirit::qi;
		namespace phoenix = boost::phoenix;

		template<typename Iter>
		struct WICalParser : qi::grammar<Iter, ICal ()>
		{
			qi::rule<Iter, ICal ()> Start_;
			qi::rule<Iter, Item ()> Item_;
			qi::rule<Iter, Field ()> Field_;
			qi::rule<Iter, std::string ()> ItemBegin_;
			qi::rule<Iter, void (std::string)> ItemEnd_;

			WICalParser ()
			: WICalParser::base_type (Start_)
			{
				auto eol = qi::lit ("\r\n");
				Field_ %= !qi::lit ("BEGIN:") >>
						!qi::lit ("END:") >>
						qi::lexeme [+(qi::char_ - ':')] >>
						':' >>
						qi::lexeme [+(qi::char_ - '\r' - '\n')] >>
						eol;

				ItemBegin_ %= "BEGIN:" >>
						qi::lexeme[+(qi::char_ - '\r' - '\n')] >>
						eol;
				ItemEnd_ = "END:" >>
						qi::string (qi::_r1) >>
						eol;

				Item_ = ItemBegin_ [phoenix::at_c<0> (qi::_val) = qi::_1] >>
						*(Field_ [phoenix::push_back (phoenix::at_c<1> (qi::_val), qi::_1)]) >>
						ItemEnd_ (phoenix::at_c<0> (qi::_val));

				Start_ %= qi::lit ("BEGIN:VCALENDAR") >>
						eol >>
						*Field_ >>
						*Item_ >>
						qi::lit ("END:VCALENDAR");
			}
		};

		template<typename Iter>
		bool ParseImpl (Iter first, Iter last)
		{
			ICal ical;

			auto before = first;
			bool r = qi::parse (first, last, WICalParser<Iter> (), ical);

			qDebug () << "parsed" << r << std::distance (before, first) << "bytes";
			qDebug () << "ICAL:" << ical.Items_.size () << ical.Fields_.size ();
			Q_FOREACH (auto field, ical.Fields_)
				qDebug () << field.Name_.c_str ();
			Q_FOREACH (auto item, ical.Items_)
			{
				qDebug () << "ITEM:" << item.Name_.c_str ();
				Q_FOREACH (auto field, item.Fields_)
					qDebug () << field.Name_.c_str ();
			}

			return true;
		}
	}

	QList<TodoItem_ptr> ICalParser::Parse (QByteArray data)
	{
		ParseImpl (data.begin (), data.end ());
		QList<TodoItem_ptr> result;
		return result;
	}
}
}
