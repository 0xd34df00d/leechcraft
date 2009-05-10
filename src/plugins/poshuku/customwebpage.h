#ifndef CUSTOMWEBPAGE_H
#define CUSTOMWEBPAGE_H
#include <boost/shared_ptr.hpp>
#include <QWebPage>
#include <QUrl>
#include <interfaces/structures.h>

class JSProxy;

class CustomWebPage : public QWebPage
{
	Q_OBJECT

	Qt::MouseButtons MouseButtons_;
	Qt::KeyboardModifiers Modifiers_;

	QUrl LoadingURL_;
	boost::shared_ptr<JSProxy> JSProxy_;
	typedef QMap<QWebFrame*, QWebHistoryItem*> Frame2History_t;
	Frame2History_t Frame2History_;
public:
	CustomWebPage (QObject* = 0);
	virtual ~CustomWebPage ();

	void SetButtons (Qt::MouseButtons);
	void SetModifiers (Qt::KeyboardModifiers);
private slots:
	void handleContentsChanged ();
	void handleDatabaseQuotaExceeded (QWebFrame*, QString);
	void handleDownloadRequested (const QNetworkRequest&);
	void handleFrameCreated (QWebFrame*);
	void handleJavaScriptWindowObjectCleared ();
	void handleGeometryChangeRequested (const QRect&);
	void handleLinkClicked (const QUrl&);
	void handleLinkHovered (const QString&, const QString&, const QString&);
	void handleLoadFinished (bool);
	void handleLoadProgress (int);
	void handleLoadStarted ();
	void handleMenuBarVisibilityChangeRequested (bool);
	void handleMicroFocusChanged ();
	void handlePrintRequested (QWebFrame*);
	void handleRepaintRequested (const QRect&);
	void handleRestoreFrameStateRequested (QWebFrame*);
	void handleSaveFrameStateRequested (QWebFrame*, QWebHistoryItem*);
	void handleScrollRequested (int, int, const QRect&);
	void handleSelectionChanged ();
	void handleStatusBarMessage (const QString&);
	void handleStatusBarVisibilityChangeRequested (bool);
	void handleToolBarVisiblityChangeRequested (bool);
	void handleUnsupportedContent (QNetworkReply*);
	void handleWindowCloseRequested ();
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

