#ifndef CUSTOMWEBPAGE_H
#define CUSTOMWEBPAGE_H
#include <QWebPage>
#include <QUrl>
#include <interfaces/structures.h>

class CustomWebPage : public QWebPage
{
	Q_OBJECT

	Qt::MouseButtons MouseButtons_;
	Qt::KeyboardModifiers Modifiers_;

	QUrl LoadingURL_;
public:
	CustomWebPage (QObject* = 0);
	virtual ~CustomWebPage ();

	void SetButtons (Qt::MouseButtons);
	void SetModifiers (Qt::KeyboardModifiers);
private slots:
	void handleDownloadRequested (const QNetworkRequest&);
	void gotUnsupportedContent (QNetworkReply*);
protected:
	virtual bool acceptNavigationRequest (QWebFrame*,
			const QNetworkRequest&, QWebPage::NavigationType);
	virtual QString chooseFile (QWebFrame*, const QString&);
	virtual QObject* createPlugin (const QString&, const QUrl&,
			const QStringList&, const QStringList&);
	virtual QWebPage* createWindow (WebWindowType);
	virtual void javaScriptAlert (QWebFrame*, const QString&);
	virtual bool javaScriptConfirm (QWebFrame*, const QString&);
	virtual void javaScriptConsoleMessage (const QString&, int, const QString&);
	virtual bool javaScriptPrompt (QWebFrame*, const QString&, const QString&, QString*);
	virtual QString userAgentForUrl (const QUrl&) const;
private:
	QWebFrame* FindFrame (const QUrl&);
signals:
	void gotEntity (const LeechCraft::DownloadEntity&);
	void loadingURL (const QUrl&);
};

#endif

