#include "cleanweb.h"
#include <typeinfo>
#include <boost/bind.hpp>
#include <QIcon>
#include <QTextCodec>
#include <QtDebug>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "subscriptionsmanager.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;
using namespace LeechCraft::Plugins::Poshuku::Plugins::CleanWeb;

void CleanWeb::Init (ICoreProxy_ptr proxy)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku_cleanweb"));
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
			"poshukucleanwebsettings.xml");
	SettingsDialog_->SetCustomWidget ("SubscriptionsManager",
			new SubscriptionsManager ());
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
	return tr ("Blocks unwanted ads.");
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
	return QStringList ("http");
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
	return Core::Instance ().CouldHandle (e);
}

void CleanWeb::Handle (DownloadEntity e)
{
	Core::Instance ().Handle (e);
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_cleanweb, CleanWeb);

