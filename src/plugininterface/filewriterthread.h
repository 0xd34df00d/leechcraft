#ifndef FILEWRITERTHREAD_H
#define FILEWRITERTHREAD_H
#include <QThread>
#include <QString>
#include <QByteArray>
#include "filewriter.h"

class FileWriterThread : public QThread
{
	Q_OBJECT
	
	QString Path_;
	QByteArray Data_;
	qint64 Position_;
	bool Overwrite_;
	FileWriter::writeid_t ID_;
public:
	FileWriterThread (const QString&, const QByteArray&, qint64, bool, FileWriter::writeid_t);
	virtual ~FileWriterThread ();

	virtual void run ();

	FileWriter::writeid_t GetID () const;
signals:
	void error (QString);
	void finished (FileWriter::writeid_t);
};

#endif

