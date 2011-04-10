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

#include "ircparsergrammar.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcParserGrammar::IrcParserGrammar ()
	{
	}

	bool IrcParserGrammar::ParseMessage (std::string message)
	{
		IrcMessageParams_.Nickname_.clear ();
		IrcMessageParams_.Command_.clear ();
		IrcMessageParams_.Message_.clear ();
		IrcMessageParams_.Parameters_.clear ();
		std::string::const_iterator first = message.begin ();
		std::string::const_iterator last = message.end ();
		return boost::spirit::qi::parse (first, last, Grammar_, IrcMessageParams_) && (first == last);
	}

	IrcMessageStruct IrcParserGrammar::GetMessageStruct () const
	{
		return IrcMessageParams_;
	}

};
};
};