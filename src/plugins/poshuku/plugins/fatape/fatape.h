/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include <QNetworkRequest>
#include <QWebPage>
#include <QWebFrame>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/poshuku/iproxyobject.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>
#include "userscript.h"

class QStandardItem;
class QStandardItemModel;

namespace LC
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

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.FatApe")

		QList<UserScript> UserScripts_;
		IProxyObject *Proxy_ = nullptr;
		ICoreProxy_ptr CoreProxy_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
		std::shared_ptr<QStandardItemModel> Model_;
	public:
		void Init (ICoreProxy_ptr);

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
		int AddScriptToManager (const UserScript& script);
	public slots:
		void hookBrowserWidgetInitialized (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookAcceptNavigationRequest (LC::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QNetworkRequest request,
				QWebPage::NavigationType type);
		void initPlugin (QObject *proxy);
	private slots:
		void handleItemChanged (QStandardItem*);
	};
}
}
}
