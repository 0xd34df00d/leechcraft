/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "rosenthal.h"
#include <QIcon>
#include <QApplication>
#include <QTextEdit>
#include <plugininterface/util.h>
#include "hunspell/hunspell.hxx"
#include "highlighter.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Rosenthal
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		const QString& locale = Util::GetLocaleName ();
#ifdef Q_WS_X11
		QString base = "/usr/local/share/myspell/";
		if (!QFile::exists (base + locale + ".aff"))
			base = "/usr/share/myspell/";
#elif defined(Q_WS_WIN32)
		base = qApp->applicationDirPath () + "/myspell/";
#endif
		QByteArray baBase = (base + locale).toLatin1 ();
		Hunspell_.reset (new Hunspell (baBase + ".aff", baBase + ".dic"));
	}

	void Plugin::SecondInit ()
	{
	}	

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Rosenthal";
	}

	void Plugin::Release ()
	{
		Hunspell_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Azoth Rosenthal";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides spellchecking for Azoth.");
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
			QObject *chatTab, QObject*, QWebView*)
	{
		QTextEdit *edit;
		QMetaObject::invokeMethod (chatTab,
				"getMsgEdit",
				Q_RETURN_ARG (QTextEdit*, edit));
		Highlighter *hl = new Highlighter (Hunspell_, edit->document ());
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_rosenthal, LeechCraft::Azoth::Rosenthal::Plugin);
