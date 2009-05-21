#include "cleanweb.h"
#include <typeinfo>
#include <boost/bind.hpp>
#include <QIcon>
#include <QTextCodec>
#include <QtDebug>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;
using namespace LeechCraft::Plugins::CleanWeb;

void CleanWeb::Init (ICoreProxy_ptr proxy)
{
	connect (&Core::Instance (),
			SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
					int*, QObject**)),
			this,
			SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
					int*, QObject**)));

	proxy->RegisterHook (HookSignature<HIDNetworkAccessManagerCreateRequest>::Signature_t (
				boost::bind (&Core::Hook,
					&Core::Instance (),
					_1,
					_2,
					_3,
					_4)));

	SettingsDialog_.reset (new XmlSettingsDialog);
	SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			":/plugins/cleanweb/settings.xml");
}

void CleanWeb::Release ()
{
}

QString CleanWeb::GetName () const
{
	return "CleanWeb";
}

QString CleanWeb::GetInfo () const
{
	return tr ("Blocks unwanted ads and other stuff frequently seen on the internets.");
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
	return QStringList ();
}

QStringList CleanWeb::Uses () const
{
	return QStringList ();
}

void CleanWeb::SetProvider (QObject*, const QString&)
{
}

boost::shared_ptr<XmlSettingsDialog> CleanWeb::GetSettingsDialog () const
{
	return SettingsDialog_;
}

bool CleanWeb::CouldHandle (const DownloadEntity& e) const
{
	if (e.Entity_.size () > 1024)
		return false;

	QString urlString = QTextCodec::codecForName ("UTF-8")->
		toUnicode (e.Entity_);
	QUrl url (urlString);
	if (url.scheme () == "abp" &&
			url.path () == "subscribe")
		return true;
	else
		return false;
}

void CleanWeb::Handle (DownloadEntity e)
{
	Core::Instance ().Handle (e);
}

Q_EXPORT_PLUGIN2 (leechcraft_cleanweb, CleanWeb);

