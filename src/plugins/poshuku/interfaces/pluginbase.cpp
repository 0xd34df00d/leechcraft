#include "pluginbase.h"
#include <stdexcept>

using namespace LeechCraft::Poshuku;

bool PluginBase::OnHandleDownloadRequested (QWebPage*, const QNetworkRequest&)
{
	return false;
}

bool PluginBase::OnGotUnsupportedContent (QWebPage*, QNetworkReply*)
{
	return false;
}

bool PluginBase::OnAcceptNavigationRequest (QWebPage*, QWebFrame*,
		const QNetworkRequest&, QWebPage::NavigationType)
{
	return false;
}

QString PluginBase::OnChooseFile (QWebPage*, QWebFrame*, const QString&)
{
	throw std::runtime_error ("We don't handle it by default.");
}

QObject* PluginBase::OnCreatePlugin (QWebPage*, const QString&, const QUrl&,
		const QStringList&, const QStringList&)
{
	throw std::runtime_error ("We don't handle it by default.");
}

QWebPage* PluginBase::OnCreateWindow (QWebPage*, QWebPage::WebWindowType)
{
	throw std::runtime_error ("We don't handle it by default.");
}

bool PluginBase::OnJavaScriptAlert (QWebPage*, QWebFrame*, const QString&)
{
	return false;
}

bool PluginBase::OnJavaScriptConfirm (QWebPage*, QWebFrame*, const QString&)
{
	throw std::runtime_error ("We don't handle it by default.");
}

bool PluginBase::OnJavaScriptConsoleMessage (QWebPage*,
		const QString&, int, const QString&)
{
	return false;
}

bool PluginBase::OnJavaScriptPrompt (QWebPage*, QWebFrame*,
		const QString&, const QString&, QString*)
{
	throw std::runtime_error ("We don't handle it by default.");
}

QString PluginBase::OnUserAgentForUrl (const QWebPage*, const QUrl&)
{
	throw std::runtime_error ("We don't handle it by default.");
}

