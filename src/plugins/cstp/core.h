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

	enum
	{
		HState
		, HURL
		, HProgress
		, HSpeed
		, HRemaining
		, HDownloading
	};
	QStringList Headers_;

	struct TaskDescr
	{
		boost::shared_ptr<Task> Task_;
		boost::shared_ptr<QFile> File_;
		QString Comment_;
	};
	typedef std::deque<TaskDescr> tasks_t;
	tasks_t RunningTasks_;
	
	explicit Core ();
public:
	virtual ~Core ();
	static Core& Instance ();
	void Release ();

	virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
	virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags (const QModelIndex&) const;
	virtual bool hasChildren (const QModelIndex&) const;
	virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
	virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent (const QModelIndex&) const;
	virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
};

#endif

