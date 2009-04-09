#include "fua.h"
#include <QStandardItemModel>
#include <QUrl>
#include <QSettings>
#include <plugininterface/proxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "settings.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft::Poshuku;
using namespace LeechCraft::Util;

void Poshuku_Fua::Init ()
{
	Browser2ID_ ["Firefox 1.5.0.4"] =
		"Mozilla/5.0 (X11; U; x86_64 Linux; en_US; rv:1.8.0.4) Gecko/20060508 Firefox/1.5.0.4";
	Browser2ID_ ["Firefox 2.0.0.8"] =
		"Mozilla/5.0 (X11; U; x86_64 Linux; en_US; rv:1.8.16) Gecko/20071015 Firefox/2.0.0.8";
	Browser2ID_ ["IE 4.01 on Windows 2000"] =
		"Mozilla/4.0 (compatible; MSIE 4.01; Windows NT 5.0)";
	Browser2ID_ ["IE 5.0 on Mac PPC"] =
		"Mozilla/4.0 (compatible; MSIE 5.0; Mac_PowerPC)";
	Browser2ID_ ["IE 5.5 on Windows 2000"] =
		"Mozilla/4.0 (compatible; MSIE 5.5; Windows NT 5.0)";
	Browser2ID_ ["IE 6.0 on Windows XP"] =
		"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";
	Browser2ID_ ["Mozilla 1.7.3 on Windows XP"] =
		"Mozilla/5.0 (Windows; U; Windows NT 5.1; en_US; rv:1.7.3) Gecko/20040916";
	Browser2ID_ ["Netscape 7.1 on Windows XP"] =
		"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-CA; rv:1.4) Gecko/20030624 Netscape/7.1 (ax)";
	Browser2ID_ ["Netscape Navigator 4.76 on Windows 95"] =
		"Mozilla/4.7 [en_US] (Win95; U)";
	Browser2ID_ ["Opera 4.03 on Windows NT 4.0"] =
		"Opera/4.03 (Windows NT 4.0; U)";
	Browser2ID_ ["Safari 2.0 on Mac OS X"] =
		"Mozilla/5.0 (Macintosh; U; PPC Mac OS X; appLanguage) AppleWebKit/412 (KHTML, like Gecko) Safari/412";
	Model_.reset (new QStandardItemModel);
	Model_->setHorizontalHeaderLabels (QStringList (tr ("Domain"))
				<< tr ("Agent")
				<< tr ("Identification string"));

	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku_FUA");
	int size = settings.beginReadArray ("Fakes");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		QString domain = settings.value ("domain").toString ();
		QString identification = settings.value ("identification").toString ();
		QList<QStandardItem*> items;
		items << new QStandardItem (domain)
			<< new QStandardItem (Browser2ID_.key (identification))
			<< new QStandardItem (identification);
		Model_->appendRow (items);
	}
	settings.endArray ();

	Settings_.reset (new Settings (Model_.get (), this));

	XmlSettingsDialog_.reset (new XmlSettingsDialog ());
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			":/poshukufuasettings.xml");
	XmlSettingsDialog_->SetCustomWidget ("Settings", Settings_.get ());
}

void Poshuku_Fua::Release ()
{
}

QString Poshuku_Fua::GetName () const
{
	return "Poshuku-FUA";
}

QString Poshuku_Fua::GetInfo () const
{
	return tr ("Allows to set fake user agents for different sites.");
}

QIcon Poshuku_Fua::GetIcon () const
{
	return QIcon ();
}

QStringList Poshuku_Fua::Provides () const
{
	return QStringList ();
}

QStringList Poshuku_Fua::Needs () const
{
	return QStringList ("webbrowser");
}

QStringList Poshuku_Fua::Uses () const
{
	return QStringList ();
}

void Poshuku_Fua::SetProvider (QObject*, const QString&)
{
}

QByteArray Poshuku_Fua::GetPluginClass () const
{
	return QByteArray (typeid (LeechCraft::Poshuku::PluginBase).name ());
}

boost::shared_ptr<XmlSettingsDialog> Poshuku_Fua::GetSettingsDialog () const
{
	return XmlSettingsDialog_;
}

void Poshuku_Fua::Init (IProxyObject*)
{
}

QString Poshuku_Fua::OnUserAgentForUrl (const QWebPage*, const QUrl& url)
{
	QString host = url.host ();
	for (int i = 0; i < Model_->rowCount (); ++i)
	{
		QStandardItem *item = Model_->item (i);
		QRegExp re (item->text (), Qt::CaseSensitive, QRegExp::Wildcard);
		if (re.exactMatch (host))
			return Model_->item (i, 2)->text ();
	}
	throw std::runtime_error ("Not found");
}

void Poshuku_Fua::Save () const
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku_FUA");
	settings.beginWriteArray ("Fakes");
	settings.remove ("");
	for (int i = 0; i < Model_->rowCount (); ++i)
	{
		settings.setArrayIndex (i);
		settings.setValue ("domain", Model_->item (i, 0)->text ());
		settings.setValue ("identification", Model_->item (i, 2)->text ());
	}
	settings.endArray ();
}

const QMap<QString, QString>& Poshuku_Fua::GetBrowser2ID () const
{
	return Browser2ID_;
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_fua, Poshuku_Fua);

