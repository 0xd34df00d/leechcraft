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
class QAction;

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
		typedef std::map<const QAbstractItemModel*, QObject*> repres2object_t;
		mutable repres2object_t Representation2Object_;
		typedef std::map<const QAction*, QAbstractItemModel*> action2model_t;
		mutable action2model_t Action2Model_;

        Core ();
    public:
        virtual ~Core ();
		static Core& Instance ();
        void Release ();

        void SetReallyMainWindow (MainWindow*);
        MainWindow* GetReallyMainWindow ();

		QAbstractItemModel* GetPluginsModel () const;
		QAbstractProxyModel* GetTasksModel () const;
		QWidget* GetControls (int) const;
		QWidget* GetAdditionalInfo (int) const;

        void DelayedInit ();
        bool ShowPlugin (IInfo::ID_t);
        void HideAll ();
        void TryToAddJob (const QString&, const QString&);

		void Activated (const QModelIndex&);
		void SetNewRow (const QModelIndex&);
        
        QPair<qint64, qint64> GetSpeeds () const;
	public slots:
		void handleProxySettings () const;
		void handlePluginAction ();
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

