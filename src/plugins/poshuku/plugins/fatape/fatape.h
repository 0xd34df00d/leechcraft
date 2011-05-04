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

#ifndef PLUGINS_POSHUKU_PLUGINS_FATAPE_FATAPE_H
#define PLUGINS_POSHUKU_PLUGINS_FATAPE_FATAPE_H
#include "userscript.h"
#include <QObject>
#include <QList>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>


namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)
	private:
		QList<UserScript> UserScripts_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QSet<QByteArray> GetPluginClasses () const;
		public slots:
		void hookJavaScriptWindowObjectCleared (LeechCraft::IHookProxy_ptr proxy,                                                                                                                                               
				QWebPage *sourcePage,                                                                                                                                                                                           
				QWebFrame *frameCleared);
	};
}
}
}

#endif
