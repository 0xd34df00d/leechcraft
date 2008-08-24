#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMap>
#include "common.h"

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
class QDockWidget;
class QModelIndex;

namespace Ui
{
	class LeechCraft;
};

namespace Main
{
    class Core;
    class PluginInfo;
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

		Ui::LeechCraft *Ui_;

        QSystemTrayIcon *TrayIcon_;
        QLabel *DownloadSpeed_, *UploadSpeed_;
        GraphWidget *DSpeedGraph_, *USpeedGraph_;

        XmlSettingsDialog *XmlSettingsDialog_;
        QList<QDockWidget*> PluginWidgets_;

        bool IsShown_;
    public:
        MainWindow (QWidget *parent = 0, Qt::WFlags flags = 0);
        virtual ~MainWindow ();
        QMenu* GetRootPluginsMenu () const;
    public slots:
        void catchError (QString);
    protected:
        virtual void closeEvent (QCloseEvent*);
    private:
        void SetTrayIcon ();
        void ReadSettings ();
        void WriteSettings ();
    private slots:
        void updateSpeedIndicators ();
        void showAboutInfo ();
        void showHideMain ();
        void hideAll ();
        void handleTrayIconActivated (QSystemTrayIcon::ActivationReason);
        void addJob ();
        void handleDownloadFinished (const QString&);
        void showSettings ();
        void handleAggregateJobsChange ();
        void cleanUp ();
		void on_PluginsTree__activated (const QModelIndex&);
    };
};

#endif

