/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_CORE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_CORE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	class GlooxProtocol;
	class GlooxCLEntry;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		boost::shared_ptr<GlooxProtocol> GlooxProtocol_;
		QObject *PluginProxy_;

		Core ();
	public:
		static Core& Instance ();

		void SecondInit ();
		void Release ();
		QList<QObject*> GetProtocols () const;

		void SetPluginProxy (QObject*);
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		void SendEntity (const Entity&);
	private:
		void LoadRoster ();
	private slots:
		void saveRoster ();
		void saveAvatarFor (GlooxCLEntry* = 0);
		void handleItemsAdded (const QList<QObject*>&);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}
}
}

#endif
