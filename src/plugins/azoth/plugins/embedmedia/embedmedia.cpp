/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "embedmedia.h"
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineView>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/sys/resourceloader.h>

namespace LC::Azoth::EmbedMedia
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::ResourceLoader loader { QStringLiteral ("azoth/embedmedia") };
		loader.AddGlobalPrefix ();
		loader.AddLocalPrefix ();

		auto embedderJS = loader.Load (QStringLiteral ("embedder.js"), true);

		if (!embedderJS || !embedderJS->isOpen ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find script file";
			return;
		}

		Script_.setSourceCode (embedderJS->readAll ());
		Script_.setName (GetUniqueID ());
		Script_.setInjectionPoint (QWebEngineScript::DocumentReady);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.EmbedMedia";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return QStringLiteral ("Azoth EmbedMedia");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Enables displaying media objects right in chat windows.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin" };
	}

	void Plugin::hookChatTabCreated (LC::IHookProxy_ptr,
			QObject*, QObject*, QWebEngineView *webView)
	{
		webView->page ()->scripts ().insert (Script_);
		/* TODO
		const auto frame = webView->page ()->mainFrame ();
		frame->evaluateJavaScript (ScriptContent_);
		connect (frame,
				&QWebFrame::initialLayoutCompleted,
				[frame, this] { frame->evaluateJavaScript (ScriptContent_); });
				*/
	}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_embedmedia, LC::Azoth::EmbedMedia::Plugin);
