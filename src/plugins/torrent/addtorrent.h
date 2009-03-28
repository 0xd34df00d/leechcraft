#ifndef ADDTORRENT_H
#define ADDTORRENT_H
#include <QDialog>
#include <QVector>
#include "ui_addtorrent.h"
#include "core.h"

class TorrentFilesModel;
namespace LeechCraft
{
	namespace Util
	{
		class TagsLineEdit;
	};
};

class AddTorrent : public QDialog, private Ui::AddTorrent
{
    Q_OBJECT

    TorrentFilesModel *FilesModel_;
public:
    AddTorrent (QWidget *parent = 0);
    void Reinit ();
    void SetFilename (const QString&);
	void SetSavePath (const QString&);
    QString GetFilename () const;
    QString GetSavePath () const;
    QVector<bool> GetSelectedFiles () const;
	Core::AddType GetAddType () const;
    QStringList GetTags () const;
	QStringList GetDefaultTags () const;
	LeechCraft::Util::TagsLineEdit* GetEdit ();
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

