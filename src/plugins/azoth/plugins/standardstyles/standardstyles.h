/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iresourceplugin.h>

class QWebEngineView;

namespace LC::Azoth
{
class IProxyObject;

namespace StandardStyles
{
	class StandardStyleSource;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IResourcePlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LC::Azoth::IResourcePlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.StandardStyles")

		StandardStyleSource *Source_ = nullptr;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QList<QObject*> GetResourceSources () const override;
	public slots:
		void initPlugin (QObject*);
		void hookThemeReloaded (const LC::IHookProxy_ptr&,
				QObject*,
				QWebEngineView*,
				QObject*);
	};
}
}
