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
#include <QWebView>
#include <QWebFrame>
#include <QFile>
#include <QMenu>
#include <QCursor>
#include <QtDebug>
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
					{
						Ui_.setupUi (this);
						setToolTip (url.toString ());
						Ui_.LoadFlash_->setToolTip (url.toString ());

						setContextMenuPolicy (Qt::CustomContextMenu);
						connect (this,
								SIGNAL (customContextMenuRequested (const QPoint&)),
								this,
								SLOT (handleContextMenu ()));
					}

					void FlashPlaceHolder::on_LoadFlash__released ()
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

						QString filename = ":/plugins/poshuku/plugins/cleanweb/resources/scripts/swap.js";
						QFile file (filename);
						file.open (QIODevice::ReadOnly);
						QString js = QString (file.readAll ()).arg (URL_.toString ());

						hide ();

						QList<QWebFrame*> frames;
						frames << view->page ()->mainFrame ();
						while (frames.size ())
						{
							QWebFrame *frame = frames.takeFirst ();
							frame->evaluateJavaScript (js);
							frames += frame->childFrames ();
						}
					}

					void FlashPlaceHolder::handleContextMenu ()
					{
						QMenu menu;
						menu.addAction (tr ("Load"),
								this,
								SLOT (on_LoadFlash__released ()));
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
				};
			};
		};
	};
};

