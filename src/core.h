#ifndef CORE_H
#define CORE_H
#include <memory>
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
class QLocalServer;

struct JobHolder
{
    IInfo *Info_;
    QAbstractItemModel *Model_;
    QAbstractItemDelegate *Delegate_;
};

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
		std::auto_ptr<QLocalServer> Server_;

        Core ();
    public:
        virtual ~Core ();
		static Core& Instance ();
        void Release ();

        void SetReallyMainWindow (MainWindow*);
        MainWindow* GetReallyMainWindow ();

		QAbstractItemModel* GetPluginsModel () const;

        void DelayedInit ();
        bool ShowPlugin (IInfo::ID_t);
        void HideAll ();
        void TryToAddJob (const QString&, const QString&);

		void Activated (const QModelIndex&);
        
        QPair<qint64, qint64> GetSpeeds () const;
        QList<JobHolder> GetJobHolders () const;
	public slots:
		void handleProxySettings () const;
    private slots:
        void handleFileDownload (const QString&, bool fromBuffer = false);
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

