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

#include "embedmedia.h"
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QTextCodec>
#include <QTextStream>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>


namespace LeechCraft
{
namespace Azoth
{
namespace EmbedMedia
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		QFile embedderJS (":/plugins/azoth/plugins/embedmedia/resources/scripts/embedder.js");

		if (!embedderJS.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open script file"
					<< embedderJS.errorString ();
			return;
		}

		QTextStream content (&embedderJS); 

		content.setCodec (QTextCodec::codecForName ("UTF-8")); 
		ScriptContent_ = content.readAll ();
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
		return "Azoth EmbedMedia";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Enables displaying media objects right in chat windows.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	void Plugin::hookChatTabCreated (LeechCraft::IHookProxy_ptr, 
			QObject*, QObject*, QWebView *webView)
	{
		webView->page ()->mainFrame ()->evaluateJavaScript (ScriptContent_);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_embedmedia, LeechCraft::Azoth::EmbedMedia::Plugin);
