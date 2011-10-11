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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H

#include <QObject>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ipluginready.h>
#include <interfaces/core/ihookproxy.h>

class QMenu;
class QGraphicsWebView;

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class Plugin : public QObject
				, public IInfo
				, public IPlugin2
				, public IHaveSettings
				, public IPluginReady
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings IPluginReady)

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		boost::shared_ptr<QTranslator> Translator_;
	public:
		// IInfo methods
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		// IPlugin2 methods
		QSet<QByteArray> GetPluginClasses () const;

		// IHaveSettings methods
		Util::XmlSettingsDialog_ptr GetSettingsDialog() const;

		//IPluginReady
		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);
	public slots:
		void initPlugin (QObject*);
		void hookMoreMenuFillEnd (LeechCraft::IHookProxy_ptr, QMenu*, QGraphicsWebView*, QObject*);
// 		void hookAddedToFavorites (LeechCraft::IHookProxy_ptr, QString, QString, QStringList);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H
