#include <QFile>
#include "filewriterthread.h"

FileWriterThread::FileWriterThread (const QString& path, const QByteArray& data, qint64 startPos, bool overwrite, FileWriter::writeid_t id)
: Path_ (path)
, Data_ (data)
, Position_ (startPos)
, Overwrite_ (overwrite)
, ID_ (id)
{
}

FileWriterThread::~FileWriterThread ()
{
    if (isRunning ())
        wait ();
}

void FileWriterThread::run ()
{
    QFile file (Path_);
    if (!file.open (QIODevice::WriteOnly | QIODevice::Append))
    {
        emit error (QString (Q_FUNC_INFO) + tr ("File open failed: ") + file.errorString ());
    }
    if (!file.seek (Position_))
    {
        emit error (QString (Q_FUNC_INFO) + tr ("Seek failed: ") + file.errorString ());
        return;
    }
    if (Overwrite_)
    {
        // We don't know what to do now, stub
    }
    if (!file.write (Data_))
    {
        emit error (QString (Q_FUNC_INFO) + tr ("Write failed: ") + file.errorString ());
        return;
    }
    emit finished (ID_);
}

FileWriter::writeid_t FileWriterThread::GetID () const
{
    return ID_;
}

