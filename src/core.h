#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <QList>
#include <QMultiMap>
#include <QFile>
#include <QPair>
#include <QString>
#include "common.h"
#include "pluginmanager.h"
#include "plugininfo.h"
#include "interfaces/interfaces.h"

class QTimer;
class QDomDocument;
class QTreeWidgetItem;

namespace Main
{
    class MainWindow;
    class Core : public QObject
    {
        Q_OBJECT

        QList<int> TasksIDPool_;
        PluginManager *PluginManager_;
        MainWindow *ReallyMainWindow_;
        QTimer *ClipboardWatchdog_;
        QString PreviousClipboardContents_;
    public:
        Core (QObject *parent = 0);
        ~Core ();
        void Release ();

        void SetReallyMainWindow (MainWindow*);
        MainWindow *GetReallyMainWindow ();

        void DelayedInit ();
        void InitTask (const QString&);
        bool ShowPlugin (IInfo::ID_t);
        void HideAll ();
        void TryToAddJob (const QString&);
        
        QPair<qint64, qint64> GetSpeeds () const;
        QList<QAbstractItemModel*> GetJobHolders () const;
    private slots:
        void handleFileDownload (const QString&);
        void handleClipboardTimer ();
    private:
        void PreparePools ();
        void FetchPlugins ();

        QVariant GetTaskData (int, int) const;
    signals:
        void error (QString);
        void pushTask (const QString&, int);
        void gotPlugin (const PluginInfo*);
        void hidePlugins ();
        void downloadFinished (const QString&);
        void gotRepresentationItem (QTreeWidgetItem*);
        void newRepresentationCycle ();
    };
};

#endif

