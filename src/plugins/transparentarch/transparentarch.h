/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_TRANSPARENTARCH_TRANSPARENTARCH_H
#define PLUGINS_TRANSPARENTARCH_TRANSPARENTARCH_H

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TransparentArch
		{
			class Plugin : public QObject
						 , public IInfo
						 , public IEntityHandler
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IEntityHandler)
				
				ICoreProxy_ptr Proxy_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
				bool CouldHandle (const LeechCraft::Entity&) const;
				void Handle (LeechCraft::Entity);
			};		
		}
	}
}

#endif // PLUGINS_TRANSPARENTARCH_TRANSPARENTARCH_H
