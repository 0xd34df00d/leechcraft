/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
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


#ifndef PLUGINS_OTZERKALU_OTZERKALU_H
#define PLUGINS_OTZERKALU_OTZERKALU_H
#include <QObject>
#include <QUrl>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/structures.h>
#include "otzerkaludownloader.h"

namespace LeechCraft
{
namespace Otzerkalu
{
	class Plugin : public QObject
				 , public IInfo
				 , public IEntityHandler
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IEntityHandler)
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		EntityTestHandleResult CouldHandle (const Entity& entity) const;
		void Handle (Entity entity);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&,
				int*, QObject**);
	};
}
}

#endif

