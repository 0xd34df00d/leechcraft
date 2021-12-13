/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rplisupportparser.h"
#include <map>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_insert_at_actor.hpp>

namespace LC::Azoth::Acetamide
{
	namespace
	{
		auto Convert (const std::map<std::string, std::string>& map)
		{
			QHash<QString, QString> result;
			for (const auto& [key, val] : map)
				result [QString::fromStdString (key)] = QString::fromStdString (val);
			return result;
		}
	}

	std::optional<QHash<QString, QString>> ParseISupportReply (const QString& reply)
	{
		using namespace boost::spirit::classic;

		std::string param;
		std::string val;
		std::map<std::string, std::string> stringParams;

		range<> ascii (char (0x01), char (0x7F));
		rule<> special = lexeme_d [ch_p ('[') | ']' | '\\' | '`' |
				'_' | '^' | '{' | '|' | '}'];
		rule<> nickname = (alpha_p | special)
				>> * (alnum_p | special | ch_p ('-'));
		rule<> nick = lexeme_d [nickname];
		rule<> value = *(alnum_p | punct_p);
		rule<> parameter = *alnum_p;
		rule<> token = !(ch_p ('-') [assign_a (val, "false")])
				>> parameter[assign_a (param)]
				>> !(ch_p ('=') >> value [assign_a (val)]);
		rule<> isuppport = nick >>
				ch_p (' ') >>
				*(eps_p[assign_a (val, "true")]
						>> token[insert_at_a (stringParams, param, val)]
						>> ch_p (' ')) >>
						str_p (":are supported") >> *(alnum_p | punct_p | blank_p);

		if (!parse (reply.toUtf8 ().constData (), isuppport).full)
			return {};
		return Convert (stringParams);
	}
}
