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
#include <interfaces/azoth/iresourceplugin.h>

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace AdiumStyles
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IResourcePlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LC::Azoth::IResourcePlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.AdiumStyles")

		ICoreProxy_ptr CoreProxy_;
		IProxyObject *Proxy_ = nullptr;
		QObjectList ResourceSources_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		QList<QObject*> GetResourceSources () const;
	public slots:
		void initPlugin (QObject*);
	};
}
}
}
