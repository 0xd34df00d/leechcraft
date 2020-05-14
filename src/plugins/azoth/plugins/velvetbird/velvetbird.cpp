/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
