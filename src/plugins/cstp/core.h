#ifndef CORE_H
#define CORE_H
#include <list>
#include <boost/intrusive_ptr.hpp>
#include <QAbstractItemModel>
#include <QStringList>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>

class Task;
class QFile;
class HistoryModel;
class RepresentationModel;
class MorphFile;

namespace boost
{
	namespace logic
	{
		class tribool;
	};
};

namespace _Local
{
	struct ObjectFinder;
	struct SpeedAccumulator;
};

namespace LeechCraft
{
	namespace Util
	{
		class HistoryModel;
	};
};

class Core : public QAbstractItemModel
{
	Q_OBJECT
	QStringList Headers_;

	friend class _Local::ObjectFinder;
	friend class _Local::SpeedAccumulator;

	struct TaskDescr
	{
		boost::intrusive_ptr<Task> Task_;
		boost::intrusive_ptr<MorphFile> File_;
		QString Comment_;
		bool ErrorFlag_;
		LeechCraft::TaskParameters Parameters_;
		quint32 ID_;
	};
	typedef std::list<TaskDescr> tasks_t;
	tasks_t ActiveTasks_;
	HistoryModel *HistoryModel_;
	RepresentationModel *RepresentationModel_;
	bool SaveScheduled_;
	QNetworkAccessManager *NetworkAccessManager_;
	std::list<quint32> IDPool_;
	QWidget *Toolbar_;
	
	explicit Core ();
public:
	enum
	{
		HURL
		, HState
		, HProgress
		, HSpeed
		, HRemaining
		, HDownloading
	};

	virtual ~Core ();
	static Core& Instance ();
	void Release ();
	LeechCraft::Util::HistoryModel* GetHistoryModel ();
	void SetToolbar (QWidget*);

	int AddTask (LeechCraft::DownloadEntity&);
	qint64 GetDone (int) const;
	qint64 GetTotal (int) const;
	bool IsRunning (int) const;
	qint64 GetTotalDownloadSpeed () const;
	bool CouldDownload (const LeechCraft::DownloadEntity&);
	QAbstractItemModel* GetRepresentationModel ();
	void SetNetworkAccessManager (QNetworkAccessManager*);
	QNetworkAccessManager* GetNetworkAccessManager () const;

	virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
	virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags (const QModelIndex&) const;
	virtual bool hasChildren (const QModelIndex&) const;
	virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
	virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
	virtual QModelIndex parent (const QModelIndex&) const;
	virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
public slots:
	void removeTriggered (int);
	void removeAllTriggered (int = -1);
	void startTriggered (int);
	void stopTriggered (int);
	void startAllTriggered (int = -1);
	void stopAllTriggered (int = -1);
private slots:
	void done (bool); 
	void updateInterface ();
	void writeSettings ();
private:
	int AddTask (const QString&, const QString&,
			const QString&, const QString&,
			LeechCraft::TaskParameters = LeechCraft::NoParameters);
	int AddTask (QNetworkReply*, const QString&,
			const QString&, const QString&,
			LeechCraft::TaskParameters = LeechCraft::NoParameters);
	int AddTask (TaskDescr&, const QString&,
			const QString&, const QString&,
			LeechCraft::TaskParameters = LeechCraft::NoParameters);
	void ReadSettings ();
	void ScheduleSave ();
	tasks_t::const_iterator FindTask (QObject*) const;
	tasks_t::iterator FindTask (QObject*);
	void Remove (tasks_t::iterator);
	void AddToHistory (tasks_t::const_iterator);
	tasks_t::const_reference TaskAt (int) const;
	tasks_t::reference TaskAt (int);
signals:
	void taskFinished (int);
	void taskRemoved (int);
	void taskError (int, IDownload::Error);
	void gotEntity (const LeechCraft::DownloadEntity&);
	void downloadFinished (const QString&);
	void error (const QString&);
	void fileExists (boost::logic::tribool*);
};

#endif

