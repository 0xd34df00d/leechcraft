#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMap>
#include "core.h"
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

namespace Main
{
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

		QMenu *File_, *Help_;
		QTreeWidget *PluginsList_;
		Main::Core *Model_;
		QSplitter *Splitter_;
		QLabel *DownloadSpeed_, *UploadSpeed_;
		LogShower *LogShower_;

		QAction *BackupSettings_, *RestoreSettings_, *ClearSettings_;
		QToolBar *PluginsToolbar_;
		QMenu *PluginsMenu_, *ToolsMenu_;

		QSystemTrayIcon *TrayIcon_;

		bool SettingsClearScheduled_, IsShown_;

		static MainWindow *Instance_;
		static QMutex *InstanceMutex_;

		MainWindow (QWidget *parent = 0, Qt::WFlags flags = 0);
		~MainWindow ();
	public:
		static MainWindow *Instance ();
	public slots:
		void catchError (QString, Errors::Severity);
	protected:
		virtual void closeEvent (QCloseEvent*);
	private:
		void SetupToolbars ();
		void SetupActions ();
		void SetupMenus ();
		void SetTrayIcon ();
		void FillMenus ();
		void MakeActions ();
		void ReadSettings ();
		void WriteSettings ();
		void InitializeMainView (const QByteArray&);
		void AddPluginToTree (const PluginInfo*);
	private slots:
		void handlePluginsListDoubleClick (QTreeWidgetItem*, int);
		void addPluginToList (const PluginInfo*);
		void pluginActionTriggered ();
		void updateSpeedIndicators ();
		void handleAddMessage (const QString&, bool);
		void backupSettings ();
		void restoreSettings ();
		void clearSettings (bool);
		void showChangelog ();
		void showAboutInfo ();
		void showHideMain ();
		void hideAll ();
		void handleTrayIconActivated (QSystemTrayIcon::ActivationReason);
	};
};

#endif

