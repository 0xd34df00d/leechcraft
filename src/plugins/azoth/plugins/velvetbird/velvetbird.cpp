/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "velvetbird.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "protomanager.h"

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		PurpleLib_.setLoadHints (QLibrary::ExportExternalSymbolsHint | QLibrary::ResolveAllSymbolsHint);
		PurpleLib_.setFileNameAndVersion ("purple", 0);
		if (!PurpleLib_.load ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to re-load libpurple, disabling VelvetBird:"
					<< PurpleLib_.errorString ();
			return;
		}

		ProtoMgr_ = new ProtoManager (proxy, this);
	}

	void Plugin::SecondInit ()
	{
		if (ProtoMgr_)
			ProtoMgr_->PluginsAvailable ();

		emit gotNewProtocols (GetProtocols ());
	}

	void Plugin::Release ()
	{
		if (ProtoMgr_)
			ProtoMgr_->Release ();

		PurpleLib_.unload ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.VelvetBird";
	}

	QString Plugin::GetName () const
	{
		return "Azoth VelvetBird";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for the protocols provided by the libpurple library.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return classes;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return ProtoMgr_ ? ProtoMgr_->GetProtoObjs () : QList<QObject*> ();
	}

	void Plugin::initPlugin (QObject*)
	{
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_velvetbird, LC::Azoth::VelvetBird::Plugin);
