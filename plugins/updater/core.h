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
								  , CheckWaiter_;

	enum CheckState
	{
		NotChecking
		, ShouldCheck
		, Checking
		, CheckedSuccessfully
		, Error
	};

	Guarded<bool> ShouldaDownload_
				, GotUpdateInfoFile_;
	Guarded<int> UpdateInfoID_;
	Guarded<QString> UpdateFilename_;
	Guarded<CheckState> CheckState_;

	struct FileRepresentation
	{
		QByteArray MD5_;
		QString Location_;
		QString URL_;
		QString Description_;
		QString Name_;
	};

	QList<FileRepresentation> Files_;
public:
	Core ();
	virtual ~Core ();
	void SetProvider (QObject*, const QString&);
	bool IsChecking () const;
public slots:
	void checkForUpdates ();
signals:
	void error (const QString&);
	void gotFile (const QString&, const QString&, const QString&);
	void finishedLoop ();
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

