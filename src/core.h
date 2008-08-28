#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QObject>
#include <QString>
#include "common.h"
#include "pluginmanager.h"
#include "interfaces/interfaces.h"

class QTimer;
class QLocalServer;
class QAbstractProxyModel;
class MergeModel;

namespace Main
{
    class MainWindow;
    class Core : public QObject
    {
        Q_OBJECT

        PluginManager *PluginManager_;
        MainWindow *ReallyMainWindow_;
        QTimer *ClipboardWatchdog_;
        QString PreviousClipboardContents_;
		std::auto_ptr<QLocalServer> Server_;
		std::auto_ptr<MergeModel> MergeModel_;

        Core ();
    public:
        virtual ~Core ();
		static Core& Instance ();
        void Release ();

        void SetReallyMainWindow (MainWindow*);
        MainWindow* GetReallyMainWindow ();

		QAbstractItemModel* GetPluginsModel () const;
		QAbstractProxyModel* GetTasksModel () const;

        void DelayedInit ();
        bool ShowPlugin (IInfo::ID_t);
        void HideAll ();
        void TryToAddJob (const QString&, const QString&);

		void Activated (const QModelIndex&);
        
        QPair<qint64, qint64> GetSpeeds () const;
	public slots:
		void handleProxySettings () const;
    private slots:
        void handleFileDownload (const QString&, bool fromBuffer = false);
        void handleClipboardTimer ();
    signals:
        void error (QString);
        void hidePlugins ();
        void downloadFinished (const QString&);
    };
};

#endif

