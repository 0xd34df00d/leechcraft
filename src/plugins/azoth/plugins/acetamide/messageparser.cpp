/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "messageparser.h"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <QString>
#include <QtDebug>
#include <util/sll/prelude.h>
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	std::optional<IrcMessageOptions> ParseMessage (const QString& msg)
	{
		using namespace boost::spirit::classic;

		std::string nickStr;
		std::string userStr;
		std::string hostStr;
		std::string commandStr;
		std::string msgStr;
		QList<std::string> opts;

		range<> ascii (char (0x01), char (0x7F));
		rule<> special = lexeme_d [ch_p ('[') | ']' | '\\' | '`' |
				'_' | '^' | '{' | '|' | '}'];
		rule<> shortname = *(alnum_p
				>> *(alnum_p || ch_p ('-'))
				>> *alnum_p);
		rule<> hostname = (shortname >> *(ch_p ('.') >> shortname)) [assign_a (hostStr)];
		rule<> nickname = (alpha_p | special)
				>> *(alnum_p | special | ch_p ('-'));
		rule<> user =  +(ascii - '\r' - '\n' - ' ' - '@' - '\0');
		rule<> host = lexeme_d [+(anychar_p - ' ')] ;
		rule<> nick = lexeme_d [nickname [assign_a (nickStr)]
				>> !(!(ch_p ('!')
						>> user [assign_a (userStr)])
						>> ch_p ('@')
						>> host [assign_a (hostStr)])];
		rule<> nospcrlfcl = (anychar_p - '\0' - '\r' - '\n' -
				' ' - ':');
		rule<> lastParam = lexeme_d [ch_p (' ')
				>> !ch_p (':')
				>> (*(ch_p (':') | ch_p (' ') | nospcrlfcl))
				[assign_a (msgStr)]];
		rule<> firsParam = lexeme_d [ch_p (' ')
				>> (nospcrlfcl >> *(ch_p (':') | nospcrlfcl))
				[push_back_a (opts)]];
		rule<> params =  *firsParam
				>> !lastParam;
		rule<> command = longest_d [(+alpha_p) |
				(repeat_p (3) [digit_p])] [assign_a (commandStr)];
		rule<> prefix = longest_d [hostname | nick];
		rule<> reply = (lexeme_d [!(ch_p (':')
				>> prefix >> ch_p (' '))]
				>> command
				>> !params
				>> eol_p);

		bool res = parse (msg.toUtf8 ().constData (), reply).full;

		if (!res)
		{
			qWarning () << "input string is not a valid IRC command"
					<< msg;
			return {};
		}

		return IrcMessageOptions
		{
			.Nick_ = QString::fromStdString (nickStr),
			.UserName_ = QString::fromStdString (userStr),
			.Host_ = QString::fromStdString (hostStr),
			.Command_ = QString::fromStdString (commandStr).toLower (),
			.Message_ = QString::fromStdString (msgStr),
			.Parameters_ = Util::Map (opts, &QString::fromStdString),
		};
	}
}
