#ifndef CORE_H
#define CORE_H
#include <QThread>
#include <QMap>
#include <QPair>
#include <QString>
#include <QList>
#include <plugininterface/guarded.h>
#include <interfaces/interfaces.h>

class QString;
class QMutex;
class QWaitCondition;
class QDomElement;

class Core : public QThread
{
    Q_OBJECT;
public:
    struct FileRep
    {
        QString MD5_;
        QString Location_;
        ulong Size_;
    };
    struct EntityRep
    {
        QString Description_;
        QString Name_;
        float Build_;
        QList<FileRep> Files_;
        QList<QString> Depends_, Provides_, Uses_;
    };
private:
    QMap<QString, QObject*> Providers_;
    QPair<QMutex*, QWaitCondition*> Waiter_
                                  , CheckWaiter_
                                  , DownloadWaiter_;

    enum CheckState
    {
        NotChecking
        , ShouldCheck
        , Checking
        , CheckedSuccessfully
        , CheckError
    };

    enum DownloadState
    {
        NotDownloading
        , ShouldDownload
        , Downloading
        , DownloadedSuccessfully
        , DownloadError
    };

    Guarded<bool> ShouldQuit_
                , GotUpdateInfoFile_;
    Guarded<int> UpdateInfoID_, DownloadFileID_;
    Guarded<QString> UpdateFilename_;
    Guarded<CheckState> CheckState_;
    Guarded<DownloadState> DownloadState_;

    QList<EntityRep> Entities_;
    QList<FileRep> ToApply_;
    QList<int> IDs2Download_;
    QMap<int, int> IDs2Pos_;
public:
    Core ();
    virtual ~Core ();
    void Release ();
    void SetProvider (QObject*, const QString&);
    bool IsChecking () const;
    bool IsDownloading () const;
public slots:
    void checkForUpdates ();
    void downloadUpdates (const QList<int>&);
signals:
    void addDownload (DirectDownloadParams);
    void error (const QString&);
    void gotFile (const Core::EntityRep&);
    void downloadedID (int);
    void finishedLoop ();
    void finishedCheck ();
    void finishedDownload ();
    void finishedApplying ();
private slots:
    void handleDownloadFinished (int);
    void handleDownloadRemoved (int);
    void handleDownloadError (int, IDirectDownload::Error);
    void handleDownloadProgressUpdated (int, int);
    void versionDownloadAdded (int);
    void fileDownloadAdded (int);
private:
    virtual void run ();
    bool Check ();
    void Download ();
    void ApplyUpdates ();
    bool Parse ();
    bool FileShouldBeDownloaded (const FileRep&) const;
    void ParseEntity (const QDomElement&);
    bool HandleSingleMirror (QObject*, const QString&);
};

Q_DECLARE_METATYPE (Core::EntityRep);

#endif

