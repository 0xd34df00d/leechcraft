#include <QMutex>
#include <QMutexLocker>
#include "filewriter.h"
#include "filewriterthread.h"

FileWriter *FileWriter::Instance_ = 0;
QMutex *FileWriter::InstanceMutex_ = new QMutex;

FileWriter::FileWriter ()
{
    for (writeid_t i = 0; i < std::numeric_limits<writeid_t>::max (); ++i)
        Pool_.first.append (i);

    Pool_.second = new QMutex;
    UnconcurrentWrites_.second = new QMutex;
    ConcurrentWrites_.second = new QMutex;
}

FileWriter::~FileWriter ()
{
    // And we should wait for all unfinished writes
    delete Pool_.second;
    delete UnconcurrentWrites_.second;
    delete ConcurrentWrites_.second;
}

FileWriter* FileWriter::Instance ()
{
    if (!Instance_)
        Instance_ = new FileWriter;
    QMutexLocker lock (InstanceMutex_);
    return Instance_;
}

FileWriter::writeid_t FileWriter::Enqueue (const QString& path, const QByteArray& data, qint64 startPos, bool concurrent, bool overwrite)
{
    Pool_.second->lock ();
    writeid_t id = Pool_.first.back ();
    Pool_.first.pop_back ();
    Pool_.second->unlock ();

    FileWriterThread *fwt = new FileWriterThread (path, data, startPos, overwrite, id);
    connect (fwt, SIGNAL (finished (writerid_t)), this, SLOT (handleFinish (writerid_t)));

    if (concurrent)
    {
        fwt->start ();
        QMutexLocker (ConcurrentWrites_.second);
        ConcurrentWrites_.first.append (fwt);
    }
    else
    {
        connect (fwt, SIGNAL (finished (writerid_t)), this, SLOT (handleFinish (writerid_t)));
        QMutexLocker (UnconcurrentWrites_.second);
        UnconcurrentWrites_.first.enqueue (fwt);
        if (UnconcurrentWrites_.first.size () == 1)
            fwt->start ();
    }

    return id;
}

bool FileWriter::IsReady (FileWriter::writeid_t id)
{
    QMutexLocker (Finished_.second);
    if (Finished_.first.contains (id))
        return true;
    else
        return false;
}

void FileWriter::handleFinish (FileWriter::writeid_t id)
{
    bool found = false;
    {
        QMutexLocker (ConcurrentWrites_.second);
        for (int i = 0; i < ConcurrentWrites_.first.size (); ++i)
            if (ConcurrentWrites_.first [i]->GetID () == id)
            {
                delete ConcurrentWrites_.first [i];
                ConcurrentWrites_.first [i] = 0;
                ConcurrentWrites_.first.remove (i);
                found = true;
                break;
            }
    }

    if (!found)
    {
        QMutexLocker (UnconcurrentWrites_.second);
        for (int i = 0; i < UnconcurrentWrites_.first.size (); ++i)
            if (UnconcurrentWrites_.first [i]->GetID () == id)
            {
                delete UnconcurrentWrites_.first [i];
                UnconcurrentWrites_.first [i] = 0;
                UnconcurrentWrites_.first.removeAt (i);
                found = true;
                break;

                UnconcurrentWrites_.first.head ()->start ();
            }
    }

    QMutexLocker (Finished_.second);
    Finished_.first.push_back (id);
}

void FileWriter::wakeupNext (FileWriter::writeid_t)
{
    UnconcurrentWrites_.second->lock ();
    UnconcurrentWrites_.first.back ()->start ();
    UnconcurrentWrites_.first.pop_back ();
    UnconcurrentWrites_.second->unlock ();
}

