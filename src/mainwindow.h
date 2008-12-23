#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QList>
#include <QModelIndex>
#include "ui_leechcraft.h"

class QLabel;
class QSplashScreen;
class QDockWidget;
class QTabWidget;
class QModelIndex;
class XmlSettingsDialog;

namespace LeechCraft
{
	namespace Util
	{
		class GraphWidget;
	};

	class Core;
	class PluginInfo;
	class PluginManagerDialog;
	class FancyPopupManager;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

		Ui::LeechCraft Ui_;

		QSystemTrayIcon *TrayIcon_;
		QLabel *DownloadSpeed_, *UploadSpeed_;
		Util::GraphWidget *DSpeedGraph_, *USpeedGraph_;

		XmlSettingsDialog *XmlSettingsDialog_;
		QList<QDockWidget*> PluginWidgets_;

		PluginManagerDialog *PluginManagerDialog_;
		FancyPopupManager *FancyPopupManager_;

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
		void SetStatusBar ();
		void SetTrayIcon ();
		void ReadSettings ();
		void WriteSettings ();
	private slots:
		void updatePanes (const QModelIndex&, const QModelIndex&);
		void updateSpeedIndicators ();
		void on_ActionAboutLeechCraft__triggered ();
		void showHideMain ();
		void handleTrayIconActivated (QSystemTrayIcon::ActivationReason);
		void on_ActionAddTask__triggered ();
		void handleDownloadFinished (const QString&);
		void on_ActionSettings__triggered ();
		void on_ActionQuit__triggered ();
		void filterParametersChanged ();
		void historyFilterParametersChanged ();
		void updateIconSet ();
		void on_ActionPluginManager__triggered ();
		void historyActivated (const QModelIndex&);
	};
};

#endif

