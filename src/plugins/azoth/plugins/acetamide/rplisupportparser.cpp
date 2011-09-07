/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "rplisupportparser.h"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <boost/spirit/include/classic_insert_at_actor.hpp>
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	using namespace boost::spirit::classic;

	RplISupportParser::RplISupportParser (IrcServerHandler *ish)
	: QObject (ish)
	, ISH_ (ish)
	{
	}

	bool RplISupportParser::ParseISupportReply (const QString& reply)
	{
		bool key = false;
		std::string param;
		std::map<std::string, bool> boolParams;
		std::map<std::string, bool>::value_type boolItem;
		std::map<std::string, std::string> stringParams;
		std::map<std::string, std::string>::value_type strItem;

		range<> ascii (char (0x01), char (0x7F));
		rule<> special = lexeme_d [ch_p ('[') | ']' | '\\' | '`' |
				'_' | '^' | '{' | '|' | '}'];
		rule<> nickname = (alpha_p | special)
				>> * (alnum_p | special | ch_p ('-'));
		rule<> nick = lexeme_d [nickname];

		rule<> value = *(alnum_p | punct_p);
		rule<> parameter = *alnum_p;
		rule<> token = longest_d [(!ch_p ('-')/*[assign_a (key, true)]*/
				>> parameter/*[assign_a (boolItem.first)]*/) |
				(parameter[assign_a (param)] >> !(ch_p ('=') >>
				value[insert_at_a(stringParams, param)]))];
		rule<> isuppport = nick
				>> ch_p (' ')
				>> *(token >> ch_p (' '))
				>> str_p(":are supported by this server");

		bool res = parse (reply.toUtf8 ().constData (), isuppport).full;
		std::map<std::string, std::string>::iterator it_start, it_end;
		it_start = stringParams.begin ();
		it_end = stringParams.end ();
		for (it_start; it_start != it_end; ++it_start)
		{
			qDebug () << Q_FUNC_INFO << QString::fromUtf8 ( (*it_start).first.c_str ())
					<< QString::fromUtf8 ( (*it_start).second.c_str ());
		}
		return res;
	}
}
}
}