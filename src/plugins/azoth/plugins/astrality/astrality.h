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

#pragma once

#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/azoth/iprotocolplugin.h>

namespace Tp
{
	class PendingOperation;
}

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	class CMWrapper;

	class Plugin : public QObject
					, public IInfo
					, public IPlugin2
					, public IProtocolPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LeechCraft::Azoth::IProtocolPlugin);

		QList<CMWrapper*> Wrappers_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		QObject* GetObject ();
		QList<QObject*> GetProtocols () const;
	public slots:
		void initPlugin (QObject*);
	private slots:
		void handleListNames (Tp::PendingOperation*);
		void handleProtoWrappers (const QList<QObject*>&);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (LeechCraft::Entity, int*, QObject**);

		void gotNewProtocols (const QList<QObject*>&);
	};
}
}
}
