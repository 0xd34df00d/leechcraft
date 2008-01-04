#ifndef FILEWRITER_H
#define FILEWRITER_H
#include <QObject>
#include <QQueue>
#include <QMultiMap>
#include <QVector>
#include <QPair>

class QMutex;
class QString;
class QByteArray;
class FileWriterThread;

class FileWriter : public QObject
{
	Q_OBJECT

public:
	typedef unsigned long int writeid_t;

	struct FileData
	{
		QByteArray Data_;
		qint64 Position_;
		bool Overwrite_;
	};
private:
	static FileWriter *Instance_;
	static QMutex *InstanceMutex_;

	FileWriter ();
	~FileWriter ();

	QPair<QVector<writeid_t>, QMutex*> Pool_;

	QPair<QQueue<FileWriterThread*>, QMutex*> UnconcurrentWrites_;
	QPair<QVector<FileWriterThread*>, QMutex*> ConcurrentWrites_;
	QPair<QVector<writeid_t>, QMutex*> Finished_;
public:
	static FileWriter* Instance ();
	writeid_t Enqueue (const QString&, const QByteArray&, qint64, bool, bool);
	bool IsReady (writeid_t);
private slots:
	void handleFinish (writeid_t);
	void wakeupNext (writeid_t);
};

#endif

