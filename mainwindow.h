#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QSettings>
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
class View;
class Core;
class PluginInfo;
class QMutex;

class MainWindow : public QMainWindow
{
    Q_OBJECT

	QMenu *File_, *Help_;
	QTreeWidget *PluginsList_;
    Core *Model_;
	QSplitter *Splitter_;
	QLabel *DownloadSpeed_, *UploadSpeed_;
	LogShower *LogShower_;

	QAction *BackupSettings_, *RestoreSettings_, *ClearSettings_;
	QToolBar *ToolToolbar_;
	QMenu *ToolsMenu_;

	bool SettingsClearScheduled_;

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
private slots:
	void handlePluginsListDoubleClick (QTreeWidgetItem*, int);
	void addPluginToList (const PluginInfo*);
	void updateSpeedIndicators ();
	void handleAddMessage (const QString&, bool);
	void backupSettings ();
	void restoreSettings ();
	void clearSettings (bool);
	void showChangelog ();
};

#endif

