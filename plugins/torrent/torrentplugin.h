#ifndef TORRENTPLUIGN_H
#define TORRENTPLUIGN_H
#include <QMainWindow>
#include <interfaces/interfaces.h>

class TorrentPlugin : public QMainWindow
					, public IInfo
					, public IWindow
					, public IDownload
{
	Q_OBJECT

	Q_INTERFACES (IInfo IWindow IDownload);

	ID_t ID_;
	bool IsShown_;
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
};

#endif

