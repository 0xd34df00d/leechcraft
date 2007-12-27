#ifndef TORRENTPLUIGN_H
#define TORRENTPLUIGN_H
#include <QMainWindow>
#include <interfaces/interfaces.h>
#include <settingsdialog/settingsdialog.h>
#include "ui_mainwindow.h"

class AddTorrent;

class TorrentPlugin : public QMainWindow
					, public IInfo
					, public IWindow
					, public IDownload
					, private Ui::MainWindow
{
	Q_OBJECT

	Q_INTERFACES (IInfo IWindow IDownload);

	ID_t ID_;
	bool IsShown_;
	SettingsDialog *SettingsDialog_;
	AddTorrent *AddTorrentDialog_;
public:
	void Init ();
	QString GetName () const;
	QString GetInfo () const;
	QString GetStatusbarMessage () const;
	IInfo& SetID (ID_t);
	ID_t GetID () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);
	void Release ();
	QIcon GetIcon () const;
	void SetParent (QWidget*);
	void ShowWindow ();
	void ShowBalloonTip ();
	qint64 GetDownloadSpeed () const;
	qint64 GetUploadSpeed () const;
	void StartAll ();
	void StopAll ();
	void StartAt (ulong);
	void StopAt (ulong);
	void DeleteAt (ulong);
protected:
	virtual void closeEvent (QCloseEvent*);
private slots:
	void on_OpenTorrent__triggered ();
	void on_RemoveTorrent__triggered ();
	void on_Resume__triggered ();
	void on_Stop__triggered ();
	void on_Preferences__triggered ();
	void showError (QString);
};

#endif

