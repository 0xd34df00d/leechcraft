#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMap>
#include "common.h"
#include "logshower.h"

class QMenu;
class QMenuBar;
class QAction;
class QToolbar;
class QSplitter;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class QSplashScreen;
class QMutex;
class XmlSettingsDialog;
class QVBoxLayout;
class GraphWidget;

namespace Main
{
    class Core;
    class PluginInfo;
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

        QSystemTrayIcon *TrayIcon_;
        QMenu *File_, *PluginsMenu_, *ActionsMenu_, *ToolsMenu_, *Help_
            , *TrayPluginsMenu_;
        QTreeWidget *PluginsList_;
        Main::Core *Model_;
        QLabel *DownloadSpeed_, *UploadSpeed_;
        GraphWidget *DSpeedGraph_, *USpeedGraph_;

        QAction *AddJob_, *Settings_, *BackupSettings_, *RestoreSettings_;
        QToolBar *Toolbar_, *PluginsToolbar_;

        XmlSettingsDialog *XmlSettingsDialog_;
        QVBoxLayout *Jobs_;

        bool IsShown_;

        static MainWindow *Instance_;
        static QMutex *InstanceMutex_;

        MainWindow (QWidget *parent = 0, Qt::WFlags flags = 0);
        ~MainWindow ();
    public:
        static MainWindow *Instance ();
        QMenu* GetRootPluginsMenu () const;
    public slots:
        void catchError (QString);
    protected:
        virtual void closeEvent (QCloseEvent*);
    private:
        void SetupToolbars ();
        void SetupMenus ();
        void SetTrayIcon ();
        void FillMenus ();
        void MakeActions ();
        void ReadSettings ();
        void WriteSettings ();
        void InitializeMainView (const QByteArray&);
        void AddPluginToTree (const PluginInfo*);
        QWidget* CreateAggregatedJobs ();
    private slots:
        void handlePluginsListDoubleClick (QTreeWidgetItem*, int);
        void addPluginToList (const PluginInfo*);
        void pluginActionTriggered ();
        void updateSpeedIndicators ();
        void backupSettings ();
        void restoreSettings ();
        void clearSettings (bool);
        void showChangelog ();
        void showAboutInfo ();
        void showHideMain ();
        void hideAll ();
        void handleTrayIconActivated (QSystemTrayIcon::ActivationReason);
        void addJob ();
        void handleDownloadFinished (const QString&);
        void showSettings ();
        void handleAggregateJobsChange ();
    };
};

#endif

