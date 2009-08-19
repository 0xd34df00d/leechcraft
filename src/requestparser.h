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

#ifndef REQUESTPARSER_H
#define REQUESTPARSER_H
#include <QObject>
#include <QStringList>
#include <interfaces/ifinder.h>

namespace LeechCraft
{
	class RequestParser : public QObject
	{
		Q_OBJECT

		Request Request_;
	public:
		RequestParser (const QString& = QString (), QObject* = 0);

		void Parse (const QString&);
		const Request& GetRequest () const;
	};
};

#endif

