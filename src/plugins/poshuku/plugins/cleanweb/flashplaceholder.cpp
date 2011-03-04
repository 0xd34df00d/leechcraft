/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "flashplaceholder.h"
#include <qwebview.h>
#include <qwebframe.h>
#include <QFile>
#include <QMenu>
#include <QCursor>
#include <QtDebug>
#include <qwebelement.h>
#include "core.h"
#include "flashonclickwhitelist.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace CleanWeb
{
	FlashPlaceHolder::FlashPlaceHolder (const QUrl& url,
			QWidget *parent)
	: QWidget (parent)
	, URL_ (url)
	, Swapping_ (false)
	{
		Ui_.setupUi (this);
		setToolTip (url.toString ());
		Ui_.LoadFlash_->setToolTip (url.toString ());
		connect (Ui_.LoadFlash_,
				SIGNAL (released ()),
				Ui_.LoadFlash_,
				SLOT (deleteLater  ()));
		connect (Ui_.LoadFlash_,
				SIGNAL (released ()),
				this,
				SLOT (handleLoadFlash  ()),
				Qt::QueuedConnection);

		setContextMenuPolicy (Qt::CustomContextMenu);
		connect (this,
				SIGNAL (customContextMenuRequested (const QPoint&)),
				this,
				SLOT (handleContextMenu ()));
	}

	bool FlashPlaceHolder::IsSwapping () const
	{
		return Swapping_;
	}

	void FlashPlaceHolder::handleLoadFlash ()
	{
		QWidget *parent = parentWidget ();
		QWebView *view = 0;
		while (parent)
		{
			if ((view = qobject_cast<QWebView*> (parent)))
				break;
			parent = parent->parentWidget ();
		}
		if (!view)
			return;

		QString selector = "%1[type=\"application/x-shockwave-flash\"]";
		QString mime = "application/futuresplash";

		hide ();

		Swapping_ = true;

		QList<QWebFrame*> frames;
		frames.append (view->page ()->mainFrame ());

		while (!frames.isEmpty ())
		{
			QWebFrame *frame = frames.takeFirst ();
			QWebElement docElement = frame->documentElement ();

			QWebElementCollection elements;
			elements.append (docElement.findAll (selector.arg ("object")));
			elements.append (docElement.findAll (selector.arg ("embed")));

			Q_FOREACH (QWebElement element, elements)
			{
				if (!element.evaluateJavaScript ("this.swapping").toBool ())
					continue;

				QWebElement substitute = element.clone ();
				substitute.setAttribute ("type", mime);
				element.replace (substitute);
			}

			frames += frame->childFrames();
		}
		Swapping_ = false;
	}

	void FlashPlaceHolder::handleHideFlash ()
	{
		QWidget *parent = parentWidget ();
		QWebView *view = 0;
		while (parent)
		{
			if ((view = qobject_cast<QWebView*> (parent)))
				break;
			parent = parent->parentWidget ();
		}
		if (!view)
			return;

		QString selector = "%1[type=\"application/x-shockwave-flash\"]";

		hide ();

		Swapping_ = true;

		QList<QWebFrame*> frames;
		frames.append (view->page ()->mainFrame ());

		while (!frames.isEmpty ())
		{
			QWebFrame *frame = frames.takeFirst ();
			QWebElement docElement = frame->documentElement ();

			QWebElementCollection elements;
			elements.append (docElement.findAll (selector.arg ("object")));
			elements.append (docElement.findAll (selector.arg ("embed")));

			Q_FOREACH (QWebElement element, elements)
			{
				if (!element.evaluateJavaScript ("this.swapping").toBool ())
					continue;

				element.removeFromDocument ();
				break;
			}

			frames += frame->childFrames();
		}
		Swapping_ = false;
	}

	void FlashPlaceHolder::handleContextMenu ()
	{
		QMenu menu;
		menu.addAction (tr ("Load"),
				this,
				SLOT (handleLoadFlash ()),
				Qt::QueuedConnection);
		menu.addSeparator ();

		QAction *addUrl = menu.addAction (tr ("Add URL to whitelist..."),
				this,
				SLOT (handleAddWhitelist ()));
		QString url = URL_.toString ();
		addUrl->setData (URL_.toString ());
		QAction *addHost = menu.addAction (tr ("Add host to whitelist..."),
				this,
				SLOT (handleAddWhitelist ()));
		QString host = URL_.host ();
		addHost->setData (URL_.host ());

		addUrl->setEnabled (!Core::Instance ()
				.GetFlashOnClickWhitelist ()->Matches (url));
		addHost->setEnabled (!Core::Instance ()
				.GetFlashOnClickWhitelist ()->Matches (host));

		menu.addSeparator ();
		menu.addAction (tr ("Hide"),
				this,
				SLOT (handleHideFlash ()),
				Qt::QueuedConnection);

		menu.exec (QCursor::pos ());
	}

	void FlashPlaceHolder::handleAddWhitelist ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
				<< "sender is not a QAction*"
				<< sender ();
			return;
		}

		Core::Instance ().GetFlashOnClickWhitelist ()->
			Add (action->data ().toString ());
	}
}
}
}
}
}
