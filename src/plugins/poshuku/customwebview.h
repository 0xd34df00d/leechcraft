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

#ifndef PLUGINS_POSHUKU_CUSTOMWEBVIEW_H
#define PLUGINS_POSHUKU_CUSTOMWEBVIEW_H
#include <qwebview.h>
#include <interfaces/structures.h>
#include <interfaces/iinfo.h>
#include "interfaces/poshukutypes.h"
#include "pageformsdata.h"

class QTimer;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class BrowserWidget;

			class CustomWebView : public QWebView
			{
				Q_OBJECT

				QList<qreal> Zooms_;
				BrowserWidget *Browser_;
				QString PreviousEncoding_;
				QTimer *ScrollTimer_;
				double ScrollDelta_;
				double AccumulatedScrollShift_;
			public:
				CustomWebView (QWidget* = 0);
				virtual ~CustomWebView ();

				void SetBrowserWidget (BrowserWidget*);
				void Load (const QString&, QString = QString ());
				void Load (const QUrl&, QString = QString ());
				void Load (const QNetworkRequest&,
						QNetworkAccessManager::Operation = QNetworkAccessManager::GetOperation,
						const QByteArray& = QByteArray ());

				/** This function is equivalent to url.toString() if the
				 * url is all in UTF-8. But if the site is in another
				 * category, QUrl::toString() returns a bad, unreadable
				 * and, moreover, unusable string. In this case, this
				 * function converts the url to its percent-encoding
				 * representation.
				 *
				 * @param[in] url The possibly non-UTF-8 URL.
				 * @return
				 */
				QString URLToProperString (const QUrl& url);
			protected:
				virtual void mousePressEvent (QMouseEvent*);
				virtual void wheelEvent (QWheelEvent*);
				virtual void contextMenuEvent (QContextMenuEvent*);
				virtual void keyReleaseEvent (QKeyEvent*);
			private:
				int LevelForZoom (qreal);
				void NavigatePlugins ();
				void NavigateHome ();
			public slots:
				void zoomIn ();
				void zoomOut ();
				void zoomReset ();
			private slots:
				void remakeURL (const QUrl&);
				void handleLoadFinished ();
				void openLinkHere ();
				void openLinkInNewTab ();
				void saveLink ();
				void subscribeToLink ();
				void bookmarkLink ();
				void copyLink ();
				void openImageHere ();
				void openImageInNewTab ();
				void saveImage ();
				void copyImage ();
				void copyImageLocation ();
				void searchSelectedText ();
				void renderSettingsChanged ();
				void handleAutoscroll ();
			signals:
				void urlChanged (const QString&);
				void gotEntity (const LeechCraft::Entity&);
				void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
				void couldHandle (const LeechCraft::Entity&, bool*);
				void addToFavorites (const QString&, const QString&);
				void printRequested (QWebFrame*);
				void closeRequested ();
				void storeFormData (const PageFormsData_t&);
				void invalidateSettings ();

				// Hook support signals
				void hookWebViewContextMenu (LeechCraft::IHookProxy_ptr,
						QWebView*, QContextMenuEvent*,
						const QWebHitTestResult&, QMenu*,
						WebViewCtxMenuStage);
			};
		};
	};
};

#endif

