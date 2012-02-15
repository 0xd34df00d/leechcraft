/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "pogooglue.h"
#include <QIcon>
#include <QMenu>
#include <QGraphicsWebView>
#include <util/util.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace Pogooglue
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_pogooglue");
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.Pogooglue";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku Pogooglue";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to search for selected text in Google in two clicks.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	void Plugin::handleGoogleIt ()
	{
		Entity e;

		QString withoutPercent = SelectedText_;
		withoutPercent.remove (QRegExp ("%%??",
				Qt::CaseInsensitive, QRegExp::Wildcard));
		QUrl testUrl (withoutPercent);
		QUrl result;
		if (testUrl.toString () == withoutPercent)
			result = QUrl::fromEncoded (SelectedText_.toUtf8 ());
		else
			result = QUrl (SelectedText_);

		if (result.scheme ().isEmpty ())
		{
			if (!SelectedText_.count (' ') && SelectedText_.count ('.'))
				result = QUrl (QString ("http://") + SelectedText_);
			else
			{
				SelectedText_.replace ('+', "%2B");
				SelectedText_.replace (' ', '+');
				QString urlStr = QString ("http://www.google.com/search?q=%2"
						"&client=leechcraft_poshuku"
						"&ie=utf-8"
						"&rls=org.leechcraft:%1")
					.arg (QLocale::system ().name ().replace ('_', '-'))
					.arg (SelectedText_);
				result = QUrl::fromEncoded (urlStr.toUtf8 ());
			}
		}

		e.Entity_ = result;
		e.Parameters_ = LeechCraft::FromUserInitiated | LeechCraft::OnlyHandle;

		emit gotEntity (e);
	}

	void Plugin::hookWebViewContextMenu (IHookProxy_ptr, QGraphicsWebView *view,
			QGraphicsSceneContextMenuEvent*, const QWebHitTestResult&, QMenu *menu,
			WebViewCtxMenuStage stage)
	{
		if (stage != WVSAfterSelectedText)
			return;

		SelectedText_ = view->page ()->selectedText ();

		if (SelectedText_.isEmpty ())
			return;

		menu->addAction (QIcon (":/plugins/poshuku/plugins/pogooglue/resources/images/google.png"),
				tr ("Google It!"),
				this,
				SLOT (handleGoogleIt ()));
	}

}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_pogooglue, LeechCraft::Poshuku::Pogooglue::Plugin);