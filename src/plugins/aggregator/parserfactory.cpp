/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include <QtDebug>
#include "parserfactory.h"
#include "parser.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			ParserFactory::ParserFactory ()
			{
			}
			
			ParserFactory& ParserFactory::Instance ()
			{
				static ParserFactory inst;
				return inst;
			}
			
			void ParserFactory::Register (Parser *parser)
			{
				Parsers_.append (parser);
			}
			
			Parser* ParserFactory::Return (const QDomDocument& doc) const
			{
				Parser *result = 0;
				for (int i = 0; i < Parsers_.size (); ++i)
					if (Parsers_.at (i)->CouldParse (doc))
					{
						result = Parsers_ [i];
						break;
					}
				return result;
			}
		};
	};
};

