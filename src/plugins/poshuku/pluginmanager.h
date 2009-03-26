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
	QString OnUserAgentForUrl (const QWebPage*, const QUrl&);
};

#endif

