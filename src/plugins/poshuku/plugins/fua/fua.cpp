#include "fua.h"
#include <QStandardItemModel>
#include <QUrl>
#include <QSettings>
#include <plugininterface/proxy.h>
#include "settings.h"

using namespace LeechCraft::Poshuku;
using namespace LeechCraft::Util;

void Poshuku_Fua::Init ()
{
	Model_.reset (new QStandardItemModel);
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
			<< new QStandardItem (BrowserToID_.key (identification))
			<< new QStandardItem (identification);
		Model_->appendRow (items);
	}
	settings.endArray ();

	Settings_.reset (new Settings (Model_.get ()));
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

void Poshuku_Fua::Init (IProxyObject*)
{
}

QByteArray Poshuku_Fua::GetPluginClass () const
{
	return QByteArray (typeid (LeechCraft::Poshuku::PluginBase).name ());
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

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_fua, Poshuku_Fua);

