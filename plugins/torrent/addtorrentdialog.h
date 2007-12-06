#ifndef ADDTORRENTDIALOG_H
#define ADDTORRENTDIALOG_H
#include "ui_addtorrentdialog.h"

class AddTorrentDialog : public QDialog, private Ui::AddTorrentDialog
{
	Q_OBJECT

	QString LastOpenDir_, LastSaveDir_;
public:
	AddTorrentDialog (const QString& lod = QString (), const QString& lsd = QString (), QWidget *parent = 0);

	void SetFiles (QList<QPair<QString, int> >);
	QStringList GetFiles2Download () const;
	QString GetFilename () const;
	QString GetDestDir () const;
	void Reinit (const QString&, const QString&);
private slots:
	void pickTorrentFile ();
	void selectDestination ();
	void toggleItem (QTreeWidgetItem*, int);
signals:
	void fileChanged (QString);
};

#endif
