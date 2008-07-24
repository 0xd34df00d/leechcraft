#ifndef CORE_H
#define CORE_H
#include <QAbstractItemModel>
#include <QStringList>
#include <deque>
#include <boost/shared_ptr.hpp>

class Task;
class QFile;

class Core : public QAbstractItemModel
{
	Q_OBJECT
	QStringList Headers_;

	struct TaskDescr
	{
		boost::shared_ptr<Task> Task_;
		boost::shared_ptr<QFile> File_;
		QString Comment_;
	};
	typedef std::deque<TaskDescr> tasks_t;
	tasks_t ActiveTasks_;
	
	explicit Core ();
public:
	enum
	{
		HState
		, HURL
		, HProgress
		, HSpeed
		, HRemaining
		, HDownloading
	};

	virtual ~Core ();
	static Core& Instance ();
	void Release ();

	void AddJob (const QString&, const QString&, const QString&, const QString&);

	virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
	virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags (const QModelIndex&) const;
	virtual bool hasChildren (const QModelIndex&) const;
	virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
	virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
	virtual QModelIndex parent (const QModelIndex&) const;
	virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

	qint64 GetDone (int) const;
	qint64 GetTotal (int) const;
	bool IsRunning (int) const;
};

#endif

