/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include <QWebEngineScript>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

class QWebEngineView;

namespace LC::Azoth::EmbedMedia
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.EmbedMedia")

		QWebEngineScript Script_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;
		QSet<QByteArray> GetPluginClasses () const override;
	private slots:
		void hookChatTabCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				QWebEngineView *webView);
	};
}
