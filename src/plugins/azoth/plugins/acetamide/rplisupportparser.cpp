/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rplisupportparser.h"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <boost/spirit/include/classic_insert_at_actor.hpp>
#include "ircserverhandler.h"

namespace LC
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

		bool res = parse (reply.toUtf8 ().constData (), isuppport).full;
		if (!res)
		{
			qWarning () << Q_FUNC_INFO
					<< "input string is not a valide IRC ISupport message"
					<< reply;
		}
		else
			ConvertFromStdMapToQMap (stringParams);

		return res;
	}

	QMap<QString, QString> RplISupportParser::GetISupportMap () const
	{
		return ISupportMap_;
	}

	void RplISupportParser::ConvertFromStdMapToQMap (const std::map<std::string, std::string>& map)
	{
		for (std::map<std::string, std::string>::const_iterator it_begin = map.begin (),
				it_end = map.end (); it_begin != it_end; ++it_begin)
			ISupportMap_.insert (QString::fromUtf8 ((*it_begin).first.c_str ()),
					QString::fromUtf8 ((*it_begin).second.c_str ()));
	}
}
}
}
