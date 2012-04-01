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

#ifndef LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_USERCOMMANDMANAGER_H
#define LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_USERCOMMANDMANAGER_H

#include <boost/function.hpp>
#include <QObject>
#include <QHash>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerHandler;
	class IrcParser;

	class UserCommandManager : public QObject
	{
		Q_OBJECT

		IrcServerHandler *ISH_;
		IrcParser *Parser_;
		QHash<QString, boost::function<void (const QStringList&)>> Command2Action_;
	public:
		UserCommandManager (IrcServerHandler*);
		QString VerifyMessage (const QString&, const QString&);
	private:
		void Init ();
	};
}
}
}

#endif // LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_USERCOMMANDMANAGER_H
