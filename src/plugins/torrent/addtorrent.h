#ifndef ADDTORRENT_H
#define ADDTORRENT_H
#include <QDialog>
#include <QVector>
#include "ui_addtorrent.h"

class QCompleter;

class AddTorrent : public QDialog, private Ui::AddTorrent
{
    Q_OBJECT
public:
    AddTorrent (QWidget *parent = 0);
    void Reinit ();
    void SetFilename (const QString&);
    QString GetFilename () const;
    QString GetSavePath () const;
    QVector<bool> GetSelectedFiles () const;
    QStringList GetTags () const;
    void SetCompleter (QCompleter*);
private slots:
    void on_TorrentBrowse__released ();
    void on_DestinationBrowse__released ();
    void on_MarkAll__released ();
    void on_UnmarkAll__released ();
    void on_MarkSelected__released ();
    void on_UnmarkSelected__released ();
    void setOkEnabled ();
private:
    void ParseBrowsed ();
signals:
    void on_TorrentFile__textChanged ();
    void on_Destination__textChanged ();
};

#endif

