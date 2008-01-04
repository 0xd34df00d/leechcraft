#ifndef ADDTORRENT_H
#define ADDTORRENT_H
#include <QDialog>
#include "ui_addtorrent.h"

class AddTorrent : public QDialog, private Ui::AddTorrent
{
	Q_OBJECT
public:
	AddTorrent (QWidget *parent = 0);
	void Reinit ();
	QString GetFilename () const;
	QString GetSavePath () const;
private slots:
	void on_TorrentBrowse__released ();
	void on_DestinationBrowse__released ();
	void setOkEnabled ();
signals:
	void on_TorrentFile__textChanged ();
	void on_Destination__textChanged ();
};

#endif

