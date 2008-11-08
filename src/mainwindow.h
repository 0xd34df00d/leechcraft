#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QList>
#include <QModelIndex>

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
class QTabWidget;
class QModelIndex;
class QAction;

namespace Ui
{
	class LeechCraft;
};

namespace Main
{
	class Core;
	class PluginInfo;
	class PluginManagerDialog;
	class FancyPopupManager;
	class SkinEngine;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

		Ui::LeechCraft *Ui_;

		QSystemTrayIcon *TrayIcon_;
		QLabel *DownloadSpeed_, *UploadSpeed_;
		GraphWidget *DSpeedGraph_, *USpeedGraph_;

		XmlSettingsDialog *XmlSettingsDialog_;
		QList<QDockWidget*> PluginWidgets_;

		PluginManagerDialog *PluginManagerDialog_;
		FancyPopupManager *FancyPopupManager_;

		SkinEngine *SkinEngine_;

		bool IsShown_;
	public:
		MainWindow (QWidget *parent = 0, Qt::WFlags flags = 0);
		virtual ~MainWindow ();
		QModelIndexList GetSelectedRows () const;
		QTabWidget* GetTabWidget () const;
	public slots:
		void catchError (QString);
	protected:
		virtual void closeEvent (QCloseEvent*);
	private:
		void SetTrayIcon ();
		void ReadSettings ();
		void WriteSettings ();
	private slots:
		void updatePanes (const QModelIndex&, const QModelIndex&);
		void updateSpeedIndicators ();
		void showAboutInfo ();
		void showHideMain ();
		void handleTrayIconActivated (QSystemTrayIcon::ActivationReason);
		void addJob ();
		void handleDownloadFinished (const QString&);
		void showSettings ();
		void handleAggregateJobsChange ();
		void cleanUp ();
		void filterParametersChanged ();
		void historyFilterParametersChanged ();
		void updateIconsSet ();
		void on_ActionPluginManager__triggered ();
		void historyActivated (const QModelIndex&);
	};
};

#endif

