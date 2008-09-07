#ifndef INTERFACES_H
#define INTERFACES_H
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QtPlugin>
#include <QFlags>
#ifndef NOGUI
#include <QIcon>
#endif
#include "structures.h"

class QAbstractItemModel;
class QAbstractItemDelegate;
class QModelIndex;

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
    virtual QIcon GetIcon () const = 0;

    virtual ~IInfo () {}
};

#ifndef NOGUI
class IWindow
{
public:
    virtual void SetParent (QWidget*) = 0;
    virtual void ShowWindow () = 0;

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

    virtual bool CouldDownload (const QString&, LeechCraft::TaskParameters) const = 0;

    virtual ~IDownload () {}
};

class IDirectDownload
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
    virtual int AddJob (const DirectDownloadParams&, LeechCraft::TaskParameters) = 0;
    virtual ~IDirectDownload () {}
};

class IPeer2PeerDownload
{
public:
    virtual int AddJob (const QString&, LeechCraft::TaskParameters) = 0;
    virtual ~IPeer2PeerDownload () {}
};

class IRemoteable
{
public:
    enum AddJobType { AJTFile, AJTString };

    virtual QList<QVariantList> GetAll () const = 0;
    virtual void AddJob (const QByteArray&, const QString&) = 0;
    virtual AddJobType GetAddJobType () const = 0;
    virtual void StartAt (int) = 0;
    virtual void StopAt (int) = 0;
    virtual void DeleteAt (int) = 0;
    virtual ~IRemoteable () {}
};

class IJobHolder
{
public:
    virtual QAbstractItemModel* GetRepresentation () const = 0;
	virtual QWidget* GetControls () const = 0;
	virtual QWidget* GetAdditionalInfo () const = 0;

    virtual ~IJobHolder () {}
};

class IImportExport
{
public:
	virtual void ImportSettings (const QByteArray&) = 0;
	virtual void ImportData (const QByteArray&) = 0;
	virtual QByteArray ExportSettings () const = 0;
	virtual QByteArray ExportData () const = 0;

	virtual ~IImportExport () {}
};

class IEmbedModel
{
public:
	virtual void ItemSelected (const QModelIndex&) = 0;

	virtual ~IEmbedModel () {}
};

class ITaggableJobs
{
public:
	virtual QStringList GetTags (int) const = 0;
	virtual void SetTags (int, const QStringList&) = 0;
};

Q_DECLARE_INTERFACE (IInfo, "org.Deviant.LeechCraft.IInfo/1.0");
#ifndef NOGUI
Q_DECLARE_INTERFACE (IWindow, "org.Deviant.LeechCraft.IWindow/1.0");
#endif
Q_DECLARE_INTERFACE (IDownload, "org.Deviant.LeechCraft.IDownload/1.0");
Q_DECLARE_INTERFACE (IDirectDownload, "org.Deviant.LeechCraft.IDirectDownload/1.0");
Q_DECLARE_INTERFACE (IPeer2PeerDownload, "org.Deviant.LeechCraft.IPeer2PeerDownload/1.0");
Q_DECLARE_INTERFACE (IRemoteable, "org.Deviant.LeechCraft.IRemoteable/1.0");
Q_DECLARE_INTERFACE (IJobHolder, "org.Deviant.LeechCraft.IJobHolder/1.0");
Q_DECLARE_INTERFACE (IImportExport, "org.Deviant.LeechCraft.IImportExport/1.0");
Q_DECLARE_INTERFACE (IEmbedModel, "org.Deviant.LeechCraft.IEmbedModel/1.0");
Q_DECLARE_INTERFACE (ITaggableJobs, "org.Deviant.LeechCraft.ITaggableJobs/1.0");

#endif

