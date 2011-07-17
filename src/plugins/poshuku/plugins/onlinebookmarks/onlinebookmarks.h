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
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>

class QWebView;
class QMenu;

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class OnlineBookmarks 	: public QObject
							, public IInfo
							, public IHaveSettings
							, public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IPlugin2)

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		boost::shared_ptr<QTranslator> Translator_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
		QSet<QByteArray> GetPluginClasses () const;
	public slots:
		void hookMoreMenuFillEnd (LeechCraft::IHookProxy_ptr, QMenu*, QWebView*, QObject*);
		void initPlugin (QObject*);
		void hookAddedToFavorites (LeechCraft::IHookProxy_ptr, QString, QString, QStringList);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H
