#include "seekthru.h"
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"

using namespace LeechCraft::Util;

void SeekThru::Init ()
{
	XmlSettingsDialog_.reset (new XmlSettingsDialog ());
	Core::Instance ();
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
	return QStringList ("opensearch");
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
	return QStringList ();
}

boost::shared_ptr<IFindProxy> SeekThru::GetProxy (const LeechCraft::Request&)
{
	return boost::shared_ptr<IFindProxy> ();
}

boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> SeekThru::GetSettingsDialog () const
{
	return XmlSettingsDialog_;
}

Q_EXPORT_PLUGIN2 (leechcraft_seekthru, SeekThru);

