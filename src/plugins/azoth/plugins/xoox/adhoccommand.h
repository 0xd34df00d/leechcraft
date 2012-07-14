/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ADHOCCOMMAND_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ADHOCCOMMAND_H
#include <QString>
#include <QStringList>
#include <QXmppDataForm.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class AdHocCommand
	{
		QString Name_;
		QString Node_;
	public:
		AdHocCommand (const QString& name, const QString& node);

		QString GetName () const;
		void SetName (const QString&);

		QString GetNode () const;
		void SetNode (const QString&);
	};

	class AdHocResult
	{
		QString Node_;
		QString SessionID_;

		QXmppDataForm Form_;

		QStringList Actions_;
	public:
		QString GetNode () const;
		void SetNode (const QString&);

		QString GetSessionID () const;
		void SetSessionID (const QString&);

		QXmppDataForm GetDataForm () const;
		void SetDataForm (const QXmppDataForm&);

		QStringList GetActions () const;
		void SetActions (const QStringList&);
	};
}
}
}

#endif
