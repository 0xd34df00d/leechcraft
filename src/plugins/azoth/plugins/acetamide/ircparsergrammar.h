/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSERGRAMMAR_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSERGRAMMAR_H

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include "localtypes.h"

namespace qi = ::boost::spirit::qi;
namespace phoenix = ::boost::phoenix;
namespace ascii = boost::spirit::ascii;

BOOST_FUSION_ADAPT_STRUCT
(
	LeechCraft::Azoth::Acetamide::IrcMessageStruct,
	(std::string, Nickname_)
	(std::string, Command_)
	(QList<std::string>, Parameters_)
	(std::string, Message_)
)

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	template <typename Iterator>
	struct Grammar : qi::grammar<Iterator, IrcMessageStruct ()>
	{
		Grammar ()
		: Grammar::base_type (start)
		{
			Msg_ =  *qi::char_;

			Nospcrlfcl_ = (qi::ascii::char_ - '\0' - '\r' - '\n' - ' ' - ':');

			FirstParameters_ = qi::raw [Nospcrlfcl_
					>> *(qi::ascii::char_ (':') | Nospcrlfcl_)];

			Params_ = FirstParameters_ % -qi::ascii::space;

			Command_ = +qi::ascii::alpha |
					(qi::repeat (3) [qi::ascii::digit]);

			Host_ = +(qi::ascii::char_ - qi::ascii::space);

			User_ = +(qi::ascii::char_ - '\r' - '\n' - ' ' - '@' - '\0');

			Special_ = qi::ascii::char_ ("[]\\`_^{|}");

			ShortName_ = *(qi::ascii::alnum | Special_)
					>> *(qi::ascii::alnum | Special_ | qi::ascii::char_ ('-'));

			HostName_ = qi::raw [ShortName_ % '.'];

			Nick_ = -(-(qi::ascii::char_ ('!')
					>> User_)
					>> qi::ascii::char_ ('@')
					>> Host_);

			Nickname_ = HostName_;

			MainRule_ = -qi::omit [':']
					>> -Nickname_
					>> -qi::omit [Nick_]
					>> -qi::omit [qi::ascii::space]
					>> Command_
					>> -qi::omit [qi::ascii::space]
					>> -Params_
					>> -qi::omit [qi::ascii::space]
					>> -qi::omit [':']
					>> -Msg_;

			start = MainRule_;
		}

		qi::rule<Iterator, IrcMessageStruct ()> start;
		qi::rule<Iterator, IrcMessageStruct ()> MainRule_;
		qi::rule<Iterator, std::string ()> Command_;
		qi::rule<Iterator, std::list<std::string> ()> Params_;
		qi::rule<Iterator, std::string ()> Msg_;
		qi::rule<Iterator, std::string ()> FirstParameters_;
		qi::rule<Iterator, std::string ()> Nospcrlfcl_;
		qi::rule<Iterator, std::string ()> Nick_;
		qi::rule<Iterator, std::string ()> Nickname_;
		qi::rule<Iterator, std::string ()> Host_;
		qi::rule<Iterator, std::string ()> User_;
		qi::rule<Iterator, std::string ()> HostName_;
		qi::rule<Iterator, std::string ()> ShortName_;
		qi::rule<Iterator> Special_;
		qi::rule<Iterator> AsciiRange_;
	};

	class IrcParserGrammar
	{
		IrcMessageStruct IrcMessageParams_;
		Grammar<std::string::const_iterator> Grammar_;
	public:
		IrcParserGrammar ();
		bool ParseMessage (std::string);
		IrcMessageStruct GetMessageStruct () const;
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSERGRAMMAR_H