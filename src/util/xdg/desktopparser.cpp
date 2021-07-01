/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "desktopparser.h"
#include <optional>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/adapted.hpp>

namespace LC::Util::XDG
{
	namespace
	{
		using FieldVal_t = boost::variant<std::string, std::vector<std::string>>;
		using Lang_t = std::optional<std::string>;
		struct Field
		{
			std::string Name_;
			Lang_t Lang_;
			FieldVal_t Val_;
		};
		using Fields_t = std::vector<Field>;

		struct Group
		{
			std::string Name_;
			Fields_t Fields_;
		};
		using Groups_t = std::vector<Group>;

		struct File
		{
			Groups_t Groups_;
		};
	}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Util::XDG::Field,
		Name_,
		Lang_,
		Val_)

BOOST_FUSION_ADAPT_STRUCT (LC::Util::XDG::Group,
		Name_,
		Fields_)

BOOST_FUSION_ADAPT_STRUCT (LC::Util::XDG::File,
		Groups_)

namespace LC::Util::XDG
{
	namespace
	{
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

				Lang_ %= '[' >> qi::lexeme [+(qi::char_ ("a-zA-Z0-9@_-"))] >> ']';

				KeyValSep_ %= *(qi::lit (' ')) >> '=' >> *(qi::lit (' '));

				LineValSingle_ %= qi::lexeme [+((qi::lit ("\\;") | (qi::char_ - ';' - '\r' - '\n')))];
				LineVal_ %= ((LineValSingle_ % ';') >> -qi::lit (";")) | LineValSingle_;

				Line_ %= qi::lexeme [+(qi::char_ ("a-zA-Z0-9-"))] >>
						-Lang_ >>
						KeyValSep_ >>
						-LineVal_ >>
						eol;

				GroupName_ %= '[' >> qi::lexeme [+(qi::char_ ("a-zA-Z0-9 "))] >> ']';

				Group_ %= GroupName_ >> eol >>
						*(Comment_ | Line_ | eol);

				Start_ %= *eol >> *Comment_ >> +Group_;

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

	auto DesktopParser::operator() (const QByteArray& data) -> Result_t
	{
		const auto& file = Parse (data.begin (), data.end ());

		Result_t result;
		for (const auto& item : file.Groups_)
		{
			Group_t group;
			for (const auto& field : item.Fields_)
			{
				if (field.Name_.empty () && field.Val_ == FieldVal_t { std::string {} })
					continue;

				const auto& values = boost::apply_visitor (ValGetter (), field.Val_);
				const auto& lang = field.Lang_ ? ToUtf8 (*field.Lang_) : QString ();
				group [ToUtf8 (field.Name_)] [lang] = values;
			}
			result [ToUtf8 (item.Name_)] = group;
		}
		return result;
	}
}
