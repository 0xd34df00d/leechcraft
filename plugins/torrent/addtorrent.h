#ifndef ADDTORRENT_H
#define ADDTORRENT_H
#include <QDialog>
#include "ui_addtorrent.h"

class AddTorrent : public QDialog, private Ui::AddTorrent
{
	Q_OBJECT
public:
	AddTorrent (QWidget *parent = 0);
private slots:
	void on_TorrentBrowse__released ();
	void on_DestinationBrowse__released ();
};

#endif

