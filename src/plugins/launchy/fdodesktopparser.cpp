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

#include "fdodesktopparser.h"
#include <iostream>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/adapted.hpp>

namespace LeechCraft
{
namespace Launchy
{
	namespace
	{
		typedef boost::variant<std::string, std::vector<std::string>> FieldVal_t;
		typedef boost::optional<std::string> Lang_t;
		struct Field
		{
			std::string Name_;
			Lang_t Lang_;
			FieldVal_t Val_;
		};
		typedef std::vector<Field> Fields_t;

		struct Group
		{
			std::string Name_;
			Fields_t Fields_;
		};
		typedef std::vector<Group> Groups_t;

		struct File
		{
			Groups_t Groups_;
		};
	}
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Launchy::Field,
		(std::string, Name_)
		(LeechCraft::Launchy::Lang_t, Lang_)
		(LeechCraft::Launchy::FieldVal_t, Val_));

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Launchy::Group,
		(std::string, Name_)
		(LeechCraft::Launchy::Fields_t, Fields_));

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Launchy::File,
		(LeechCraft::Launchy::Groups_t, Groups_));

namespace LeechCraft
{
namespace Launchy
{
	namespace
	{
		namespace ascii = boost::spirit::ascii;
		namespace qi = boost::spirit::qi;
		namespace phoenix = boost::phoenix;

		template<typename Iter>
		struct Parser : qi::grammar<Iter, File ()>
		{
			qi::rule<Iter, File ()> Start_;
			qi::rule<Iter, Group ()> Group_;
			qi::rule<Iter, std::string ()> GroupName_;
			qi::rule<Iter, std::string ()> Lang_;
			qi::rule<Iter, void ()> KeyValSep_;
			qi::rule<Iter, std::string ()> LineValSingle_;
			qi::rule<Iter, FieldVal_t ()> LineVal_;
			qi::rule<Iter, Field ()> Line_;
			qi::rule<Iter, void ()> Comment_;

			Parser ()
			: Parser::base_type (Start_)
			{
				auto eol = qi::lit ("\n");
				Comment_ %= qi::lit ("#") >> *(qi::char_ - '\r' - '\n') >> eol;

				Lang_ %= '[' >> qi::lexeme [+(qi::char_ ("a-zA-Z0-9"))] >> ']';

				KeyValSep_ %= *(qi::lit (' ')) >> '=' >> *(qi::lit (' '));

				LineValSingle_ %= qi::lexeme [+((qi::lit ("\\;") | (qi::char_ - ';' - '\r' - '\n')))];
				LineVal_ %= +(LineValSingle_ >> ';') | LineValSingle_;

				Line_ %= qi::lexeme [+(qi::char_ ("a-zA-Z0-9-"))] >>
						-Lang_ >>
						KeyValSep_ >>
						LineVal_ >>
						eol;

				GroupName_ %= '[' >> qi::lexeme [+(qi::char_ ("a-zA-Z0-9 "))] >> ']';

				Group_ %= GroupName_ >> eol >>
						*(Comment_ | Line_);

				Start_ %= *Comment_ >> +Group_;

				qi::on_error<qi::fail> (Start_,
						std::cout << phoenix::val ("Error! Expecting") << qi::_4
								<< phoenix::val (" here: \"") << phoenix::construct<std::string> (qi::_3, qi::_2)
								<< phoenix::val ("\"") << std::endl);
			}
		};

		template<typename Iter>
		File Parse (Iter begin, Iter end)
		{
			File res;
			qi::parse (begin, end, Parser<Iter> (), res);
			return res;
		}

		QString ToUtf8 (const std::string& str)
		{
			return QString::fromUtf8 (str.c_str ());
		}

		struct ValGetter : public boost::static_visitor<QStringList>
		{
			QStringList operator() (const std::string& str) const
			{
				return QStringList (ToUtf8 (str));
			}

			QStringList operator() (const std::vector<std::string>& vec) const
			{
				QStringList result;
				std::transform (vec.begin (), vec.end (), std::back_inserter (result), ToUtf8);
				return result;
			}
		};
	}

	FDODesktopParser::Result_t FDODesktopParser::operator() (const QByteArray& data)
	{
		const auto& file = Parse (data.begin (), data.end ());

		Result_t result;
		for (const auto& item : file.Groups_)
		{
			Group_t group;
			for (const auto& field : item.Fields_)
			{
				const auto& values = boost::apply_visitor (ValGetter (), field.Val_);
				const auto& lang = field.Lang_ ? ToUtf8 (*field.Lang_) : QString ();
				group [ToUtf8 (field.Name_)] [lang] = values;
			}
			result [ToUtf8 (item.Name_)] = group;
		}
		return result;
	}
}
}
