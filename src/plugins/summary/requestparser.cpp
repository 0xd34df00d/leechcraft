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

#include "requestparser.h"
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			RequestParser::RequestParser (const QString& string, QObject *parent)
			: QObject (parent)
			{
				Parse (string);
			}

			void RequestParser::Parse (QString request)
			{
				Request_ = Request ();
				Request_.CaseSensitive_ = false;

				QRegExp rx ("(ca|category):\"(.+)\"");
				rx.setMinimal (true);
				int rxpos = rx.indexIn (request);
				if (rxpos == -1)
					Request_.Category_ = "downloads";
				else
				{
					QStringList caps = rx.capturedTexts ();
					Request_.Category_ = caps.at (2);
					request.remove (caps.at (0));
				}

				QStringList tokens = request.split (' ', QString::SkipEmptyParts);
				foreach (QString token, tokens)
				{
					int index = token.indexOf (':');
					// Skip if there is no :, if it is first or last.
					if (index == -1 ||
							index == 0 ||
							token.size () - 1 == index)
						continue;

					// If : is escaped, erase the \ symbol and skip.
					if (token [index - 1] == '\\')
					{
						token.remove (index - 1, 1);
						continue;
					}

					tokens.removeAll (token + ' ');
					// This is for the case where the token is the last token in the
					// request, so there is no token + ' '
					tokens.removeAll (token);

					QString key = token.left (index).toLower ();
					QString value = token.mid (index + 1);

					if (key == "pl" ||
							key == "plugin")
						Request_.Plugin_ = value;
					else if (key == "cs" ||
							key == "casesensitive")
						Request_.CaseSensitive_ = (value.toLower () == "true" ||
								value.toLower () == "on");
					else if (key == "t" ||
							key == "type")
					{
						if (value == "f" ||
								value == "fixed")
							Request_.Type_ = Request::RTFixed;
						else if (value == "w" ||
								value == "wildcard")
							Request_.Type_ = Request::RTWildcard;
						else if (value == "r" ||
								value == "regexp")
							Request_.Type_ = Request::RTRegexp;
						else if (value == "t" ||
								value == "tag")
							Request_.Type_ = Request::RTTag;
					}
					else
						Request_.Params_ [key] = value;
				}
				Request_.String_ = tokens.join (" ");
			}

			const Request& RequestParser::GetRequest () const
			{
				return Request_;
			}
		};
	};
};

