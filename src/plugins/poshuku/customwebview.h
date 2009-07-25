#ifndef PLUGINS_POSHUKU_CUSTOMWEBVIEW_H
#define PLUGINS_POSHUKU_CUSTOMWEBVIEW_H
#include <QWebView>
#include <interfaces/structures.h>
#include "pageformsdata.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class CustomWebView : public QWebView
			{
				Q_OBJECT

				QList<qreal> Zooms_;
			public:
				CustomWebView (QWidget* = 0);
				virtual ~CustomWebView ();

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
			private:
				int LevelForZoom (qreal);
			public slots:
				void zoomIn ();
				void zoomOut ();
				void zoomReset ();
			private slots:
				void remakeURL (const QUrl&);
				void openLinkHere ();
				void openLinkInNewTab ();
				void saveLink ();
				void bookmarkLink ();
				void copyLink ();
				void openImageHere ();
				void openImageInNewTab ();
				void saveImage ();
				void copyImage ();
				void copyImageLocation ();
				void searchSelectedText ();
			signals:
				void urlChanged (const QString&);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void addToFavorites (const QString&, const QString&);
				void printRequested (QWebFrame*);
				void closeRequested ();
				void storeFormData (const PageFormsData_t&);
			};
		};
	};
};

#endif

