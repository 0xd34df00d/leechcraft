#ifndef CUSTOMWEBVIEW_H
#define CUSTOMWEBVIEW_H
#include <QWebView>

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
private slots:
	void remakeURL (const QUrl&);
	void handleNewThumbs ();
signals:
	void urlChanged (const QString&);
	void gotEntity (const QByteArray&);
};

#endif

