#include "seekthru.h"
#include <QMessageBox>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "searcherslist.h"

using namespace LeechCraft::Util;

void SeekThru::Init ()
{
	connect (&Core::Instance (),
			SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
					int*, QObject**)),
			this,
			SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
					int*, QObject**)));
	connect (&Core::Instance (),
			SIGNAL (error (const QString&)),
			this,
			SLOT (handleError (const QString&)));

	XmlSettingsDialog_.reset (new XmlSettingsDialog ());
	XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
			":/seekthrusettings.xml");
	XmlSettingsDialog_->SetCustomWidget ("SearchersList", new SearchersList);
}

void SeekThru::Release ()
{
	XmlSettingsDialog_.reset ();
}

QString SeekThru::GetName () const
{
	return "SeekThru";
}

QString SeekThru::GetInfo () const
{
	return tr ("Search via OpenSearch-aware search providers.");
}

QIcon SeekThru::GetIcon () const
{
	return QIcon ();
}

QStringList SeekThru::Provides () const
{
	return QStringList ("search");
}

QStringList SeekThru::Needs () const
{
	return QStringList ("http");
}

QStringList SeekThru::Uses () const
{
	return QStringList ();
}

void SeekThru::SetProvider (QObject*, const QString&)
{
}

QStringList SeekThru::GetCategories () const
{
	return Core::Instance ().GetCategories ();
}

IFindProxy_ptr SeekThru::GetProxy (const LeechCraft::Request& r)
{
	return Core::Instance ().GetProxy (r);
}

boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> SeekThru::GetSettingsDialog () const
{
	return XmlSettingsDialog_;
}

void SeekThru::handleError (const QString& error)
{
	QMessageBox::critical (0,
			tr ("Error"),
			error);
}

Q_EXPORT_PLUGIN2 (leechcraft_seekthru, SeekThru);

