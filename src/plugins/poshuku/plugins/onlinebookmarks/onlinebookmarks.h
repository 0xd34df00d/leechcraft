/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H

#include <QObject>
#include <QMenu>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ipluginready.h>
#include <interfaces/core/ihookproxy.h>

class QWebView;

namespace LC
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

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.OnlineBookmarks")

		Util::XmlSettingsDialog_ptr SettingsDialog_;
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
		void hookMoreMenuFillEnd (LC::IHookProxy_ptr, QMenu*, QObject*);
	signals:
		void gotEntity (const LC::Entity&);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H
