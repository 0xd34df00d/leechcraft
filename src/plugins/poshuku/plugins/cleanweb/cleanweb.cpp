#include "cleanweb.h"
#include <typeinfo>
#include <QtDebug>

using namespace LeechCraft::Poshuku;
using namespace LeechCraft::Poshuku::Plugins;

void CleanWeb::Init (ICoreProxy_ptr)
{
}

void CleanWeb::Release ()
{
}

QString CleanWeb::GetName () const
{
	return "Poshuku CleanWeb";
}

QString CleanWeb::GetInfo () const
{
	return tr ("Blocks unwanted popups, ads and other stuff frequently seen in the internets.");
}

QIcon CleanWeb::GetIcon () const
{
	return QIcon ();
}

QStringList CleanWeb::Provides () const
{
	return QStringList ();
}

QStringList CleanWeb::Needs () const
{
	return QStringList ("webbrowser");
}

QStringList CleanWeb::Uses () const
{
	return QStringList ();
}

void CleanWeb::SetProvider (QObject*, const QString&)
{
}

QByteArray CleanWeb::GetPluginClass () const
{
	return QByteArray (typeid (PluginBase).name ());
}

void CleanWeb::Init (IProxyObject*)
{
}

bool CleanWeb::OnAcceptNavigationRequest (QWebPage *page, QWebFrame *frame,
		const QNetworkRequest& req, QWebPage::NavigationType type)
{
	return false;
}


Q_EXPORT_PLUGIN2 (leechcraft_poshuku_cleanweb, CleanWeb);

