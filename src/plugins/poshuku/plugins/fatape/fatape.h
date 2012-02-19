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

#ifndef PLUGINS_POSHUKU_PLUGINS_FATAPE_FATAPE_H
#define PLUGINS_POSHUKU_PLUGINS_FATAPE_FATAPE_H
#include "userscript.h"
#include <QObject>
#include <QList>
#include <QStandardItemModel>
#include <QNetworkRequest>
#include <QWebPage>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/iproxyobject.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>

class QTranslator;

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		std::shared_ptr<QTranslator> Translator_;
		QList<UserScript> UserScripts_;
		IProxyObject *Proxy_;
		ICoreProxy_ptr CoreProxy_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
		std::shared_ptr<QStandardItemModel> Model_;
	public:
		void Init (ICoreProxy_ptr);

		void AddScriptToManager (const UserScript& script);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QSet<QByteArray> GetPluginClasses () const;
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
		void EditScript (int scriptIndex);
		void DeleteScript (int scriptIndex);
		void SetScriptEnabled(int scriptIndex, bool value);
	public slots:
		void hookInitialLayoutCompleted (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame);
		void hookAcceptNavigationRequest (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QNetworkRequest request,
				QWebPage::NavigationType type);
		void initPlugin (QObject *proxy);
	};
}
}
}

#endif
