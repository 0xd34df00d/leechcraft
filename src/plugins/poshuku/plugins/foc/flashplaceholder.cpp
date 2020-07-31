/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flashplaceholder.h"
#include <qwebview.h>
#include <qwebframe.h>
#include <QFile>
#include <QMenu>
#include <QCursor>
#include <QtDebug>
#include <qwebelement.h>
#include "flashonclickwhitelist.h"

namespace LC
{
namespace Poshuku
{
namespace FOC
{
	FlashPlaceHolder::FlashPlaceHolder (const QUrl& url, FlashOnClickWhitelist *wl, QWidget *parent)
	: QWidget (parent)
	, WL_ (wl)
	, URL_ (url)
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

	void FlashPlaceHolder::PerformWithElements (const std::function<void (QWebElement)>& function)
	{
		QWidget *parent = parentWidget ();
		QWebView *view = nullptr;
		while (parent)
		{
			if ((view = qobject_cast<QWebView*> (parent)))
				break;

			parent = parent->parentWidget ();
		}
		if (!view)
			return;

		const QString selector { "%1[type=\"application/x-shockwave-flash\"]" };

		hide ();

		Swapping_ = true;

		QList<QWebFrame*> frames { view->page ()->mainFrame () };

		while (!frames.isEmpty ())
		{
			const auto frame = frames.takeFirst ();
			auto docElement = frame->documentElement ();

			auto elements = docElement.findAll (selector.arg ("object")) +
					docElement.findAll (selector.arg ("embed"));

			for (auto element : elements)
			{
				if (!element.evaluateJavaScript ("this.swapping").toBool ())
					continue;

				function (element);
				break;
			}

			frames += frame->childFrames();
		}
		Swapping_ = false;
	}

	void FlashPlaceHolder::handleLoadFlash ()
	{
		PerformWithElements ([] (QWebElement element)
				{
					auto substitute = element.clone ();
					substitute.setAttribute ("type", "application/futuresplash");
					element.replace (substitute);
				});
	}

	void FlashPlaceHolder::handleHideFlash ()
	{
		PerformWithElements ([] (QWebElement element)
				{
					element.removeFromDocument ();
				});
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

		addUrl->setEnabled (!WL_->Matches (url));
		addHost->setEnabled (!WL_->Matches (host));

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

		WL_->Add (action->data ().toString ());
	}
}
}
}
