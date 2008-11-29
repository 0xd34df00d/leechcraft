#ifndef CUSTOMWEBPAGE_H
#define CUSTOMWEBPAGE_H
#include <QWebPage>

class CustomWebPage : public QWebPage
{
	Q_OBJECT
public:
	CustomWebPage (QObject* = 0);
	virtual ~CustomWebPage ();
private slots:
	void handleDownloadRequested (const QNetworkRequest&);
};

#endif

