/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/ipluginadaptor.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/iscriptloader.h>

namespace LC
{
namespace Qrosp
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPluginAdaptor
				 , public IEntityHandler
				 , public IScriptLoader
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPluginAdaptor IEntityHandler IScriptLoader)

		LC_PLUGIN_METADATA ("org.LeechCraft.Qrosp")
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Provides () const;

		QList<QObject*> GetPlugins ();

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		IScriptLoaderInstance_ptr CreateScriptLoaderInstance (const QString&);
	};
}
}
