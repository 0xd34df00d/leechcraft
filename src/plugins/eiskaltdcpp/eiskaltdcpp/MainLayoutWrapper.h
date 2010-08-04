#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QDockWidget>
#include <QLabel>
#include <QList>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QCloseEvent>
#include <QShowEvent>
#include <QTabBar>
#include <QToolBar>
#include <QHash>
#include <QSessionManager>
#include <QShortcut>
#include <QKeySequence>

#include <interfaces/iinfo.h>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ConnectionManager.h"
#include "dcpp/DownloadManager.h"
#include "dcpp/LogManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/TimerManager.h"
#include "dcpp/UploadManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Download.h"
#include "dcpp/version.h"

#include "ArenaWidget.h"
#include "HistoryInterface.h"
#include "Func.h"

#include "ui_UIAbout.h"

using namespace dcpp;

class FavoriteHubs;
class DownloadQueue;
class ToolBar;
class MainLayoutWrapper;

class QProgressBar;

class MainLayoutWrapperCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1210);

    MainLayoutWrapperCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~MainLayoutWrapperCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class About:
        public QDialog,
        public Ui::UIAbout
{
Q_OBJECT

public:
    About(QWidget *parent): QDialog(parent){ setupUi(this); }
};

class MainLayoutWrapper:
        public QMainWindow,
		public IInfo,
        private LogManagerListener,
        private TimerManagerListener,
        private QueueManagerListener
{
		Q_OBJECT
		Q_INTERFACES (IInfo)

		static MainLayoutWrapper *S_StaticThis;
    public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();

		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QStringList Provides () const;
		QStringList Needs () const;
		QStringList Uses () const;
		void SetProvider (QObject*, const QString&);
		QIcon GetIcon () const;

        typedef QList<QAction*> ActionList;
        typedef QList<ArenaWidget*> ArenaWidgetList;
        typedef QMap<ArenaWidget*, QWidget*> ArenaWidgetMap;

		static MainLayoutWrapper* getInstance ();

        /** Allow widget to be mapped on arena*/
        void addArenaWidget(ArenaWidget*);
        /** Disallow widget to be mapped on arena*/
        void remArenaWidget(ArenaWidget*);
        /** Show widget on arena */
        void mapWidgetOnArena(ArenaWidget*);
        /** Remove widget from arena*/
        void remWidgetFromArena(ArenaWidget*);

        /** */
        void setStatusMessage(QString);

        /** */
        void newHubFrame(QString, QString);

        /** */
        void addArenaWidgetOnToolbar(ArenaWidget*, bool keepFocus = false);
        /** */
        void remArenaWidgetFromToolbar(ArenaWidget*);

        /** */
        void browseOwnFiles();

        /** */
        void redrawToolPanel();

        /** */
        void startSocket();

        /** */
        void autoconnect();

        /** */
        void parseCmdLine();
        /** */
        void parseInstanceLine(QString);

        /** */
        void retranslateUi();
    public slots:
        void slotChatClear();

    protected:
        virtual void closeEvent(QCloseEvent*);
        virtual void showEvent(QShowEvent *);
        virtual void customEvent(QEvent *);
        virtual bool eventFilter(QObject *, QEvent *);

    private slots:
        void slotFileOpenLogFile();
        void slotFileBrowseFilelist();
        void slotFileBrowseOwnFilelist();
        void slotFileRefreshShare();
        void slotFileHashProgress();
        void slotHubsReconnect();
        void slotHubsFavoriteHubs();
        void slotHubsPublicHubs();
        void slotHubsFavoriteUsers();
        void slotToolsDownloadQueue();
        void slotToolsFinishedDownloads();
        void slotToolsFinishedUploads();
        void slotToolsSpy();
        void slotToolsAntiSpam();
        void slotToolsIPFilter();
        void slotToolsSearch();
        void slotToolsSettings();
        void slotToolsTransfer(bool);
        void slotWidgetsToggle();
        void slotQC();
        void slotHideWindow();
        void slotHideProgressSpace();

        void slotCloseCurrentWidget();

        void slotUnixSignal(int);

        void slotFindInChat();
        void slotChatDisable();

        void slotAboutClient();
        void slotAboutQt();

    private:
		void ConstructAsConstructor ();
		void ReleaseAsClosed ();
		void ReleaseAsAfterExec ();
		void ReleaseAsDestructor ();

        /** LogManagerListener */
        virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string&) throw();
        /** TimerManagerListener */
        virtual void on(dcpp::TimerManagerListener::Second, uint32_t) throw();
        /** QueueManagerListener */
        virtual void on(dcpp::QueueManagerListener::Finished, QueueItem*, const std::string&, int64_t) throw();
        //
        void showShareBrowser(dcpp::UserPtr, QString, QString);

        // Interface setup functions
        void init();
        void loadSettings();
        void saveSettings();

        void initActions();
        void initMenuBar();
        void initStatusBar();
        void initToolbar();
        void initHotkeys();

        void toggleSingletonWidget(ArenaWidget *a);

        void updateStatus(QMap<QString,QString>);

        // Widgets
        QDockWidget *arena;
        QDockWidget *transfer_dock;

        ToolBar *tBar;//for tabs
        ToolBar *fBar;//for actions

        QLabel *statusLabel;
        QLabel *statusDSPLabel;
        QLabel *statusUSPLabel;
        QLabel *statusDLabel;
        QLabel *statusULabel;

        QLabel *statusTRLabel;
        QLabel *msgLabel;
        QProgressBar *progressSpace;

        QMenu   *menuFile;
        QAction *fileFileListBrowser;
        QAction *fileFileListBrowserLocal;
        QAction *fileFileListRefresh;
        QAction *fileHashProgress;
        QAction *fileOpenLogFile;
        QAction *fileHideWindow;

        QMenu   *menuHubs;
        QAction *hubsHubReconnect;
        QAction *hubsQuickConnect;
        QAction *hubsFavoriteHubs;
        QAction *hubsPublicHubs;
        QAction *hubsFavoriteUsers;

        QMenu   *menuTools;
        QAction *toolsSearch;
        QAction *toolsTransfers;
        QAction *toolsDownloadQueue;
        QAction *toolsFinishedDownloads;
        QAction *toolsFinishedUploads;
        QAction *toolsSpy;
        QAction *toolsAntiSpam;
        QAction *toolsIPFilter;
        QAction *toolsHideProgressSpace;
        QAction *toolsOptions;

        QShortcut *ctrl_pgup;
        QShortcut *ctrl_pgdown;
        QShortcut *ctrl_w;

        QAction *chatDisable;
        QAction *findInChat;
        QAction *chatClear;

        QMenu *menuWidgets;
        QList<QAction*> menuWidgetsActions;
        QHash<QAction*, ArenaWidget*> menuWidgetsHash;

        QMenu   *menuAbout;
        QAction *aboutClient;
        QAction *aboutQt;

        ActionList toolBarActions;
        ActionList fileMenuActions;
        ActionList hubsMenuActions;
        ActionList toolsMenuActions;
        ArenaWidgetList arenaWidgets;
        ArenaWidgetMap arenaMap;

        HistoryInterface<QWidget*> history;
};

#endif //MAINWINDOW_H_

