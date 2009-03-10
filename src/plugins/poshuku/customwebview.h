#ifndef CUSTOMWEBVIEW_H
#define CUSTOMWEBVIEW_H
#include <QWebView>
#include <interfaces/structures.h>

class CustomWebView : public QWebView
{
	Q_OBJECT
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
private slots:
	void remakeURL (const QUrl&);
	void handleNewThumbs ();
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
signals:
	void urlChanged (const QString&);
	void gotEntity (const LeechCraft::DownloadEntity&);
	void addToFavorites (const QString&, const QString&);
};

#endif

