#ifndef MOVETORRENTFILES_H
#define MOVETORRENTFILES_H
#include <QDialog>
#include "ui_movetorrentfiles.h"

class MoveTorrentFiles : public QDialog
{
	Q_OBJECT

	Ui::MoveTorrentFiles Ui_;
public:
	MoveTorrentFiles (const QString&, QWidget* = 0);
	QString GetNewLocation () const;
private slots:
	void on_Browse__released ();
};

#endif

