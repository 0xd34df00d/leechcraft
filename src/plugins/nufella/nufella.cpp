#include "nufella.h"
#include <plugininterface/util.h>

void Nufella::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("nufella"));
}

void Nufella::Release ()
{
}

QString Nufella::GetName () const
{
	return "Nufella";
}

QString Nufella::GetInfo () const
{
	return tr ("Just another Gnutella implementation");
}

QStringList Nufella::Provides () const
{
	return QStringList ("gnutella");
}

QStringList Nufella::Needs () const
{
	return QStringList ();
}

QStringList Nufella::Uses () const
{
	return QStringList ();
}

void Nufella::SetProvider (QObject*, const QString&)
{
}

QIcon Nufella::GetIcon () const
{
	return QIcon ();
}

qint64 Nufella::GetDownloadSpeed () const
{
	return 0;
}

qint64 Nufella::GetUploadSpeed () const
{
	return 0;
}

void Nufella::StartAll ()
{
}

void Nufella::StopAll ()
{
}

bool Nufella::CouldDownload (const QByteArray& data, LeechCraft::TaskParameters params) const
{
	return false;
}

int Nufella::AddJob (const LeechCraft::DownloadParams& downParams, LeechCraft::TaskParameters taskParams)
{
	return -1;
}

QAbstractItemModel* Nufella::GetRepresentation () const
{
	return 0;
}

LeechCraft::Util::HistoryModel* Nufella::GetHistory () const
{
	return 0;
}

QWidget* Nufella::GetControls () const
{
	return 0;
}

QWidget* Nufella::GetAdditionalInfo () const
{
	return 0;
}

void Nufella::ItemSelected (const QModelIndex&)
{
}

Q_EXPORT_PLUGIN2 (leechcraft_nufella, Nufella);

