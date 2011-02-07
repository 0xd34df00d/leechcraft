/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_IRESOURCEPLUGIN_H
#define PLUGINS_AZOTH_INTERFACES_IRESOURCEPLUGIN_H
#include <QtGlobal>
#include <QString>
#include <QList>
#include <QMap>
#include <QImage>

class QAbstactItemModel;

namespace LeechCraft
{
namespace Azoth
{
	class IResourceSource
	{
	public:
		virtual ~IResourceSource () {}
		
		virtual QAbstractItemModel* GetOptionsModel () const = 0;
	};
	
	class ISmileResourceSource : public IResourceSource
	{
	public:
		virtual ~ISmileResourceSource () {}

		virtual QList<QString> GetSmileStrings (const QString& pack) const = 0;

		virtual QMap<QImage, QString> GetReprImages (const QString& pack) const = 0;

		virtual QByteArray GetImage (const QString& pack, const QString& string) const = 0;
	};

	class IResourcePlugin
	{
	public:
		virtual ~IResourcePlugin () {}
		
		virtual QList<QObject*> GetResourceSources () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISmileResourceSource,
		"org.Deviant.LeechCraft.Azoth.ISmileResourceSource/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IResourcePlugin,
		"org.Deviant.LeechCraft.Azoth.IResourcePlugin/1.0");

#endif
