#ifndef INTERFACES_H
#define INTERFACES_H
#include <QString>
#include <QStringList>
#include <QtPlugin>
#ifndef NOGUI
#include <QIcon>
#endif
#include "../settingsdialog/settingsiteminfo.h"
#include "structures.h"

class Proxy;
class MainWindow;

class ISettings
{
public:
    virtual SettingsItemInfo GetInfoFor (const QString&) const = 0;
    virtual ~ISettings () {}
};

// Plugin-related

class IInfo
{
public:
    typedef unsigned long int ID_t;
    virtual void Init () = 0;
    virtual QString GetName () const = 0;
    virtual QString GetInfo () const = 0;
    virtual QString GetStatusbarMessage () const = 0;
    virtual IInfo& SetID (ID_t) = 0;
    virtual ID_t GetID () const = 0;
    virtual QStringList Provides () const = 0;
    virtual QStringList Needs () const = 0;
    virtual QStringList Uses () const = 0;
    virtual void SetProvider (QObject*, const QString&) = 0;
    virtual void PushMainWindowExternals (const MainWindowExternals&) = 0;
    virtual void Release () = 0;
    virtual ~IInfo () {}
};

#ifndef NOGUI
class IWindow
{
public:
    virtual QIcon GetIcon () const = 0;

    virtual void SetParent (QWidget*) = 0;

    virtual void ShowWindow () = 0;
    virtual void ShowBalloonTip () = 0;

    virtual ~IWindow () {}
};
#endif

class IDownload
{
public:
    typedef unsigned long int JobID_t;

    virtual qint64 GetDownloadSpeed () const = 0;
    virtual qint64 GetUploadSpeed () const = 0;

    virtual void StartAll () = 0;
    virtual void StopAll () = 0;

    virtual bool CouldDownload (const QString&) const = 0;
    virtual void AddJob (const QString&) = 0;

    virtual ~IDownload () {}
};

class IDirectDownload : public IDownload
{
public:
    enum Error
    {
    NoError
    , ErrorNetwork
    , ErrorNotFound
    , ErrorDenied
    , ErrorOther
    };

    virtual ~IDirectDownload () {}
};

class IPeer2PeerDownload : public IDownload
{
public:
    virtual ~IPeer2PeerDownload () {};
};

Q_DECLARE_INTERFACE (ISettings, "org.Deviant.LeechCraft.ISettings/1.0");
Q_DECLARE_INTERFACE (IInfo, "org.Deviant.LeechCraft.IInfo/1.0");
#ifndef NOGUI
Q_DECLARE_INTERFACE (IWindow, "org.Deviant.LeechCraft.IWindow/1.0");
#endif
Q_DECLARE_INTERFACE (IDownload, "org.Deviant.LeechCraft.IDownload/1.0");
Q_DECLARE_INTERFACE (IDirectDownload, "org.Deviant.LeechCraft.IDirectDownload/1.0");
Q_DECLARE_INTERFACE (IPeer2PeerDownload, "org.Deviant.LeechCraft.IPeer2PeerDownload/1.0");

#endif

