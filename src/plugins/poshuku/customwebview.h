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
#include "pageformsdata.h"

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
			public:
				CustomWebView (QWidget* = 0);
				virtual ~CustomWebView ();

				void SetBrowserWidget (BrowserWidget*);
				void Load (const QString&, QString = QString ());
				void Load (const QUrl&, QString = QString ());
				void Load (const QNetworkRequest&,
						QNetworkAccessManager::Operation = QNetworkAccessManager::GetOperation,
						const QByteArray& = QByteArray ());
			protected:
				virtual QWebView* createWindow (QWebPage::WebWindowType);
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
			signals:
				void urlChanged (const QString&);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void couldHandle (const LeechCraft::DownloadEntity&, bool*);
				void addToFavorites (const QString&, const QString&);
				void printRequested (QWebFrame*);
				void closeRequested ();
				void storeFormData (const PageFormsData_t&);
				void invalidateSettings ();
			};
		};
	};
};

#endif

