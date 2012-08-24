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

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/monocle/ibackendplugin.h>
#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>

namespace LeechCraft
{
namespace Monocle
{
namespace Seen
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IBackendPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LeechCraft::Monocle::IBackendPlugin)

		ddjvu_context_t *Context_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		bool CanLoadDocument (const QString&);
		IDocument_ptr LoadDocument (const QString&);
	private slots:
		void checkMessageQueue ();
	};
}
}
}
