#include <QtGui>
#include "torrentplugin.h"
#include "core.h"
#include "addtorrent.h"
#include "settingsmanager.h"

void TorrentPlugin::Init ()
{
	setupUi (this);
	IsShown_ = false;
	SettingsDialog_ = new SettingsDialog (this);
	SettingsDialog_->RegisterObject (SettingsManager::Instance ());
	AddTorrentDialog_ = new AddTorrent (this);
	connect (Core::Instance (), SIGNAL (error (QString)), this, SLOT (showError (QString)));
	TorrentView_->setModel (Core::Instance ());
}

QString TorrentPlugin::GetName () const
{
	return windowTitle ();
}

QString TorrentPlugin::GetInfo () const
{
	return tr ("BitTorrent client using rb-libtorrent.");
}

QString TorrentPlugin::GetStatusbarMessage () const
{
	return QString ("");
}

IInfo& TorrentPlugin::SetID (IInfo::ID_t id)
{
	ID_ = id;
	return *this;
}

IInfo::ID_t TorrentPlugin::GetID () const
{
	return ID_;
}

QStringList TorrentPlugin::Provides () const
{
	return QStringList ("bittorrent") << "resume";
}

QStringList TorrentPlugin::Needs () const
{
	return QStringList ();
}

QStringList TorrentPlugin::Uses () const
{
	return QStringList ();
}

void TorrentPlugin::SetProvider (QObject*, const QString&)
{
}

void TorrentPlugin::Release ()
{
	Core::Instance ()->Release ();
	SettingsManager::Instance ()->Release ();
}

QIcon TorrentPlugin::GetIcon () const
{
	return windowIcon ();
}

void TorrentPlugin::SetParent (QWidget *w)
{
	setParent (w);
}

void TorrentPlugin::ShowWindow ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void TorrentPlugin::ShowBalloonTip ()
{
}

qint64 TorrentPlugin::GetDownloadSpeed () const
{
	return 0;
}

qint64 TorrentPlugin::GetUploadSpeed () const
{
	return 0;
}

void TorrentPlugin::StartAll ()
{
}

void TorrentPlugin::StopAll ()
{
}

void TorrentPlugin::StartAt (ulong)
{
}

void TorrentPlugin::StopAt (ulong)
{
}

void TorrentPlugin::DeleteAt (ulong)
{
}

void TorrentPlugin::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

void TorrentPlugin::on_OpenTorrent__triggered ()
{
	if (AddTorrentDialog_->exec () == QDialog::Rejected)
		return;

	QString filename = AddTorrentDialog_->GetFilename (),
			path = AddTorrentDialog_->GetSavePath ();
	Core::Instance ()->AddFile (filename, path);
}

void TorrentPlugin::on_RemoveTorrent__triggered ()
{
}

void TorrentPlugin::on_Resume__triggered ()
{
}

void TorrentPlugin::on_Stop__triggered ()
{
}

void TorrentPlugin::on_Preferences__triggered ()
{
	SettingsDialog_->show ();
	SettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void TorrentPlugin::showError (QString e)
{
	QMessageBox::warning (this, tr ("Error!"), e);
}

Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);

