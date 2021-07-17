/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "icalparser.h"
#include <optional>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/phoenix/fusion.hpp>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"

namespace LC
{
namespace Otlozhu
{
	namespace
	{
		typedef boost::variant<std::string> FieldVal_t;
		typedef std::optional<FieldVal_t> MaybeFieldVal_t;
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

			std::optional<FieldVal_t> operator[] (const std::string& fName) const
			{
				auto pos = std::find_if (Fields_.begin (), Fields_.end (),
						[&fName] (const auto& field) { return field.Name_ == fName; });
				if (pos == Fields_.end ())
					return {};
				else
					return pos->Val_;
			}
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

BOOST_FUSION_ADAPT_STRUCT (LC::Otlozhu::Field,
		Name_,
		Val_);

BOOST_FUSION_ADAPT_STRUCT (LC::Otlozhu::Item,
		Name_,
		Fields_);

BOOST_FUSION_ADAPT_STRUCT (LC::Otlozhu::ICal,
		Fields_,
		Items_);

namespace LC
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
						qi::lexeme [+(qi::char_ - '\r' - '\n')] >>
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
				qi::on_error<qi::fail> (Start_,
						std::cout << phoenix::val ("Error! Expecting") << qi::_4
								<< phoenix::val (" here: \"") << phoenix::construct<std::string> (qi::_3, qi::_2)
								<< phoenix::val ("\"") << std::endl);
			}
		};

		template<typename Iter>
		ICal ParseImpl (Iter first, Iter last)
		{
			ICal ical;
			qi::parse (first, last, WICalParser<Iter> (), ical);
			return ical;
		}

		QString AsQString (const MaybeFieldVal_t& val)
		{
			if (!val)
				return QString ();
			return QString::fromUtf8 (boost::get<std::string> (*val).c_str ());
		}

		QString AsQStrings (const std::vector<MaybeFieldVal_t>& vals)
		{
			QString res;
			for (auto i = vals.begin (), end = vals.end (); i != end; ++i)
			{
				res = AsQString (*i);
				if (!res.isEmpty ())
					break;
			}
			return res;
		}

		QDateTime AsQDateTime (const MaybeFieldVal_t& val)
		{
			const QString& fmtStr = "yyyyMMddTHHmmss";
			const auto& str = AsQString (val).left (fmtStr.size ());
			return QDateTime::fromString (str, fmtStr);
		}

		int AsInt (const MaybeFieldVal_t& val)
		{
			return AsQString (val).toInt ();
		}
	}

	QList<TodoItem_ptr> ICalParser::Parse (QByteArray data)
	{
		const auto& ical = ParseImpl (data.begin (), data.end ());

		QList<TodoItem_ptr> result;

		for (const auto& item : ical.Items_)
		{
			if (item.Name_ != "VTODO")
				continue;

			const auto& id = item ["UID"];
			if (!id)
			{
				qWarning () << Q_FUNC_INFO << "no UID";
				continue;
			}

			TodoItem_ptr todo (new TodoItem (AsQString (id)));
			todo->SetCreatedDate (AsQDateTime (item ["DTSTAMP"]));
			todo->SetDueDate (AsQDateTime (item ["DUE"]));
			todo->SetTitle (AsQStrings ({ item ["SUMMARY"], item ["DESCRIPTION"] }));
			todo->SetComment (AsQStrings ({ item ["COMMENT"], item ["DESCRIPTION"] }));
			todo->SetPercentage (AsInt (item ["PERCENT-COMPLETE"]));

			const auto& tags = AsQString (item ["CATEGORIES"]).split (',', Qt::SkipEmptyParts);
			auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
			todo->SetTagIDs (Util::Map (tags, [tm] (const QString& tag) { return tm->GetID (tag); }));

			result << todo;
		}

		return result;
	}
}
}
