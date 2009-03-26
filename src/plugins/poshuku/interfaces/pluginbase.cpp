#include "pluginbase.h"
#include <stdexcept>

using namespace LeechCraft::Poshuku;

bool PluginBase::OnHandleDownloadRequested (QWebPage*, const QNetworkRequest&)
{
	return false;
}

bool PluginBase::OnAcceptNavigationRequest (QWebPage*, QWebFrame*,
		const QNetworkRequest&, QWebPage::NavigationType)
{
	return false;
}

bool PluginBase::OnGotUnsupportedContent (QWebPage*, QNetworkReply*)
{
	return false;
}

QString PluginBase::OnUserAgentForUrl (const QWebPage*, const QUrl&)
{
	throw std::runtime_error ("We don't handle it by default.");
}

