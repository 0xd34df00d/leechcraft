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

/*
	Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *																		 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	 *
 *   (at your option) any later version.								   *
 *																		 *
 ***************************************************************************
*/
#ifndef XMLSETTINGSDIALOG_SCRIPTER_H
#define XMLSETTINGSDIALOG_SCRIPTER_H
#include <memory>
#include <QVariant>
#include <QDomElement>
#include "settings.h"

class QScriptEngine;

namespace LeechCraft
{
	class Scripter
	{
		std::auto_ptr<QScriptEngine> Engine_;
		std::auto_ptr<Settings> Settings_;
		QDomElement Container_;
	public:
		Scripter (const QDomElement&);

		QStringList GetOptions ();
		QString HumanReadableOption (const QString&);
	private:
		QString GetScript (const QDomElement&) const;
		void FeedRequiredClasses () const;
		void Reset ();
	};
};

#endif

