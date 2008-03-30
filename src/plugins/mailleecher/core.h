#ifndef CORE_H
#define CORE_H
#include <QObject>

class WorkingThread;

class Core : public QObject
{
    Q_OBJECT

    WorkingThread *Thread_;
public:
    static Core* Instance ();
    Core (QObject *parent = 0);

    void StartDownload (const QString&, int, const QString&, const QString&, const QString&);
    void Abort ();
private slots:
    void handleFinish (bool);
signals:
    void log (const QString&);
    void error (const QString&);
    void mailProgress (int);
    void dataProgress (int);
    void totalMail (int);
    void totalData (int);
    void finished (bool);
};

#endif

