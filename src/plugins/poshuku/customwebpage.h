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
	void filteredContentsChanged ();
	void filteredDatabaseQuotaExceeded (QWebFrame*, QString);
	void filteredDownloadRequested (const QNetworkRequest&);
	void filteredFrameCreated (QWebFrame*);
	void filteredGeometryChangeRequested (const QRect&);
	void filteredLinkClicked (const QUrl&);
	void filteredLinkHovered (const QString&, const QString&, const QString&);
	void filteredLoadFinished (bool);
	void filteredLoadProgress (int);
	void filteredLoadStarted ();
	void filteredMenuBarVisibilityChangeRequested (bool);
	void filteredMicroFocusChanged ();
	void filteredPrintRequested (QWebFrame*);
	void filteredRepaintRequested (const QRect&);
	void filteredRestoreFrameStateRequested (QWebFrame*);
	void filteredSaveFrameStateRequested (QWebFrame*, QWebHistoryItem*);
	void filteredScrollRequested (int, int, const QRect&);
	void filteredSelectionChanged ();
	void filteredStatusBarMessage (const QString&);
	void filteredStatusBarVisibilityChangeRequested (bool);
	void filteredToolBarVisiblityChangeRequested (bool);
	void filteredUnsupportedContent (QNetworkReply*);
	void filteredWindowCloseRequested ();
	void gotEntity (const LeechCraft::DownloadEntity&);
	void loadingURL (const QUrl&);
};

#endif

