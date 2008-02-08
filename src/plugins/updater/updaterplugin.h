#ifndef UPDATERPLUGIN_H
#define UPDATERPLUGIN_H
#include <QMainWindow>
#include <interfaces/interfaces.h>
#include "core.h"

class SettingsDialog;
class QTreeWidget;
class QToolBar;
class QLabel;
class XmlSettingsDialog;

class UpdaterPlugin : public QMainWindow
                    , public IInfo
                    , public IWindow
{
    Q_OBJECT
    Q_INTERFACES (IInfo IWindow);

    bool IsShown_;
    ID_t ID_;
    SettingsDialog *SettingsDialog_;
    XmlSettingsDialog *XmlSettingsDialog_;
    QTreeWidget *Updates_;
    QToolBar *MainToolbar_;
    QAction *CheckForUpdates_, *DownloadUpdates_, *Settings_;
    QLabel *SizeLabel_;
    bool SaveChangesScheduled_;

    enum Columns
    {
        ColumnSize = 0
        , ColumnLocation = 1
    };

    enum Roles
    {
        RoleSize = 40
        , RoleID = 41
    };

    Core *Core_;
public:
    virtual void Init ();
    virtual ~UpdaterPlugin ();
    virtual QString GetName () const;
    virtual QString GetInfo () const;
    virtual QString GetStatusbarMessage () const;
    virtual IInfo& SetID (ID_t);
    virtual ID_t GetID () const;
    virtual QStringList Provides () const;
    virtual QStringList Needs () const;
    virtual QStringList Uses () const;
    virtual void SetProvider (QObject*, const QString&);
    virtual void PushMainWindowExternals (const MainWindowExternals&);
    virtual void Release ();
    virtual QIcon GetIcon () const;
    virtual void SetParent (QWidget*);
    virtual void ShowWindow ();
    virtual void ShowBalloonTip ();
public slots:
    void handleHidePlugins ();
protected:
    virtual void closeEvent (QCloseEvent*);
private:
    void SetupInterface ();
    void SetupMainWidget ();
    void SetupToolbars ();
    void SetupActions ();
    void SetupMenus ();
    void ReadSettings ();
private slots:
    void saveSettings ();
    void showSettings ();
    void initCheckForUpdates ();
    void initDownloadUpdates ();
    void addFile (const Core::EntityRep&);
    void handleError (const QString&);
    void handleDownloadedID (int);
    void setActionsEnabled ();
    void handleFinishedCheck ();
    void handleFinishedDownload ();
    void handleFinishedApplying ();
    void updateStatusbar ();
};

#endif

