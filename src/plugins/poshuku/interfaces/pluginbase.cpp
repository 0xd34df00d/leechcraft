#include "pluginbase.h"
#include <stdexcept>

using namespace LeechCraft::Poshuku;

bool PluginBase::HandleContentsChanged (QWebPage*)
{
	return false;
}

bool PluginBase::HandleDatabaseQuotaExceeded (QWebPage*, QWebFrame*, QString)
{
	return false;
}

bool PluginBase::HandleDownloadRequested (QWebPage*, const QNetworkRequest&)
{
	return false;
}

bool PluginBase::HandleFrameCreated (QWebPage*, QWebFrame*)
{
	return false;
}

bool PluginBase::HandleGeometryChangeRequested (QWebPage*, const QRect&)
{
	return false;
}

bool PluginBase::HandleLinkClicked (QWebPage*, const QUrl&)
{
	return false;
}

bool PluginBase::HandleLinkHovered (QWebPage*, const QString&,
		const QString&, const QString&)
{
	return false;
}

bool PluginBase::HandleLoadFinished (QWebPage*, bool)
{
	return false;
}

bool PluginBase::HandleLoadProgress (QWebPage*, int)
{
	return false;
}

bool PluginBase::HandleLoadStarted (QWebPage*)
{
	return false;
}

bool PluginBase::HandleMenuBarVisibilityChangeRequested (QWebPage*, bool)
{
	return false;
}

bool PluginBase::HandleMicroFocusChanged (QWebPage*)
{
	return false;
}

bool PluginBase::HandlePrintRequested (QWebPage*, QWebFrame*)
{
	return false;
}

bool PluginBase::HandleRepaintRequested (QWebPage*, const QRect&)
{
	return false;
}

bool PluginBase::HandleRestoreFrameStateRequested (QWebPage*, QWebFrame*)
{
	return false;
}

bool PluginBase::HandleSaveFrameStateRequested (QWebPage*, QWebFrame*, QWebHistoryItem*)
{
	return false;
}

bool PluginBase::HandleScrollRequested (QWebPage*, int, int, const QRect&)
{
	return false;
}

bool PluginBase::HandleSelectionChanged (QWebPage*)
{
	return false;
}

bool PluginBase::HandleStatusBarMessage (QWebPage*, const QString&)
{
	return false;
}

bool PluginBase::HandleStatusBarVisibilityChangeRequested (QWebPage*, bool)
{
	return false;
}

bool PluginBase::HandleToolBarVisibilityChangeRequested (QWebPage*, bool)
{
	return false;
}

bool PluginBase::HandleUnsupportedContent (QWebPage*, QNetworkReply*)
{
	return false;
}

bool PluginBase::HandleWindowCloseRequested (QWebPage*)
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

