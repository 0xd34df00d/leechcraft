#ifndef CORE_H
#define CORE_H
#include <QThread>
#include <QMap>
#include <QPair>
#include <QString>
#include <QList>
#include <plugininterface/guarded.h>
#include <interfaces/interfaces.h>

class QString;
class QMutex;
class QWaitCondition;
class QDomElement;

class Core : public QThread
{
	Q_OBJECT;

	QMap<QString, QObject*> Providers_;
	QPair<QMutex*, QWaitCondition*> Waiter_
								  , CheckWaiter_
								  , DownloadWaiter_;

	enum CheckState
	{
		NotChecking
		, ShouldCheck
		, Checking
		, CheckedSuccessfully
		, CheckError
	};

	enum DownloadState
	{
		NotDownloading
		, ShouldDownload
		, Downloading
		, DownloadedSuccessfully
		, DownloadError
	};

	Guarded<bool> ShouldQuit_
				, GotUpdateInfoFile_;
	Guarded<int> UpdateInfoID_;
	Guarded<QString> UpdateFilename_;
	Guarded<CheckState> CheckState_;
	Guarded<DownloadState> DownloadState_;

	struct FileRepresentation
	{
		QByteArray MD5_;
		QString Location_;
		QString URL_;
		QString Description_;
		QString Name_;
		ulong Size_;
	};

	QList<FileRepresentation> Files_;
public:
	Core ();
	virtual ~Core ();
	void Release ();
	void SetProvider (QObject*, const QString&);
	bool IsChecking () const;
	bool IsDownloading () const;
public slots:
	void checkForUpdates ();
	void downloadUpdates ();
signals:
	void error (const QString&);
	void gotFile (const QString&, const QString&, ulong, const QString&);
	void finishedLoop ();
	void finishedCheck ();
private slots:
	void handleDownloadFinished (int);
	void handleDownloadRemoved (int);
	void handleDownloadError (int, IDirectDownload::Error);
	void handleDownloadProgressUpdated (int, int);
private:
	virtual void run ();
	bool Check ();
	void Download ();
	bool Parse ();
	void CollectFiles (QDomElement&);
	bool HandleSingleMirror (IDirectDownload*, const QString&);
};

#endif

