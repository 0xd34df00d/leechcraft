#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include <vector>
#include "interfaces/pluginbase.h"

class PluginManager : public LeechCraft::Poshuku::PluginBase
{
	Q_OBJECT

	std::vector<LeechCraft::Poshuku::PluginBase_ptr> Plugins_;
public:
	PluginManager (QObject* = 0);

	void AddPlugin (QObject*);

	void Init (LeechCraft::Poshuku::IProxyObject*);
	bool OnHandleDownloadRequested (QWebPage*, const QNetworkRequest&);
	bool OnGotUnsupportedContent (QWebPage*, QNetworkReply*);
	bool OnAcceptNavigationRequest (QWebPage*, QWebFrame*,
			const QNetworkRequest&, QWebPage::NavigationType);
	QString OnChooseFile (QWebPage*, QWebFrame*, const QString&);
	QObject* OnCreatePlugin (QWebPage*, const QString&, const QUrl&,
			const QStringList&, const QStringList&);
	QWebPage* OnCreateWindow (QWebPage*, QWebPage::WebWindowType);
	bool OnJavaScriptAlert (QWebPage*, QWebFrame*, const QString&);
	bool OnJavaScriptConfirm (QWebPage*, QWebFrame*, const QString&);
	bool OnJavaScriptConsoleMessage (QWebPage*, const QString&,
			int, const QString&);
	bool OnJavaScriptPrompt (QWebPage*, QWebFrame*, const QString&,
			const QString&, QString*);
	QString OnUserAgentForUrl (const QWebPage*, const QUrl&);
};

#endif

