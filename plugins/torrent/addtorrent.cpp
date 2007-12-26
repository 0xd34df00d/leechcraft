#include <QFileDialog>
#include "addtorrent.h"

AddTorrent::AddTorrent (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
}

void AddTorrent::on_TorrentBrowse__released ()
{
	QString filename = QFileDialog::getOpenFileName (this, tr ("Select torrent file"), "", tr ("Torrents (*.torrent);;All files (*.*)"));
	if (filename.isEmpty ())
		return;
	else
		TorrentFile_->setText (filename);
}

