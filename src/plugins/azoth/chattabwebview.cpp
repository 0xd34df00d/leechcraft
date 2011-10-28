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

#include "chattabwebview.h"
#include <QContextMenuEvent>
#include <QWebHitTestResult>
#include <QPointer>
#include <QMenu>
#include <util/util.h>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	ChatTabWebView::ChatTabWebView (QWidget *parent)
	: QWebView (parent)
	{
	}

	void ChatTabWebView::contextMenuEvent (QContextMenuEvent *e)
	{
		QPointer<QMenu> menu (new QMenu (this));
		const QWebHitTestResult r = page ()->
				mainFrame ()->hitTestContent (e->pos ());

		if (!r.linkUrl ().isEmpty ())
		{
			const QUrl& url = r.linkUrl ();

			menu->addAction (tr ("Open"),
					this,
					SLOT (handleOpenLink ()))->setData (url);
			menu->addAction (tr ("Save..."),
					this,
					SLOT (handleSaveLink ()))->setData (url);
			menu->addAction (pageAction (QWebPage::CopyLinkToClipboard));
			menu->addSeparator ();
		}

		if (!page ()->selectedText ().isEmpty ())
			menu->addAction (pageAction (QWebPage::Copy));

		if (!r.imageUrl ().isEmpty ())
			menu->addAction (pageAction (QWebPage::CopyImageToClipboard));

		if (settings ()->testAttribute (QWebSettings::DeveloperExtrasEnabled))
			menu->addAction (pageAction (QWebPage::InspectElement));

		menu->exec (mapToGlobal (e->pos ()));
		if (menu)
			delete menu;
	}

	void ChatTabWebView::handleOpenLink ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		const Entity& e = Util::MakeEntity (action->data (),
				QString (),
				static_cast<TaskParameters> (OnlyHandle | FromUserInitiated));
		Core::Instance ().SendEntity (e);
	}

	void ChatTabWebView::handleSaveLink ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		Entity e = Util::MakeEntity (action->data (),
				QString (),
				FromUserInitiated);
		e.Additional_ ["AllowedSemantics"] = QStringList ("fetch") << "save";
		Core::Instance ().SendEntity (e);
	}
}
}
