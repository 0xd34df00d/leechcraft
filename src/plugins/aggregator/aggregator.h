#ifndef AGGREGATOR_H
#define AGGREGATOR_H
#include <memory>
#include <interfaces/iinfo.h>
#include <interfaces/iembedtab.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/structures.h>
#include <QWidget>
#include <QItemSelection>

class QSystemTrayIcon;
class QTranslator;
class QToolBar;
class ItemBucket;
class Enclosure;

class IDownload;

struct Aggregator_Impl;

class Aggregator : public QWidget
                 , public IInfo
                 , public IEmbedTab
				 , public IHaveSettings
				 , public IJobHolder
				 , public IEntityHandler
				 , public IHaveShortcuts
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab IHaveSettings IJobHolder IEntityHandler IHaveShortcuts)

	Aggregator_Impl *Impl_;
public:
	virtual ~Aggregator ();
    void Init ();
    void Release ();
    QString GetName () const;
    QString GetInfo () const;
    QStringList Provides () const;
    QStringList Needs () const;
    QStringList Uses () const;
    void SetProvider (QObject*, const QString&);
    QIcon GetIcon () const;
	QWidget* GetTabContents ();
	boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;
	QAbstractItemModel* GetRepresentation () const;
	LeechCraft::Util::HistoryModel* GetHistory () const;
	void ItemSelected (const QModelIndex&);
	bool CouldHandle (const LeechCraft::DownloadEntity&) const;
	void Handle (LeechCraft::DownloadEntity);
	void SetShortcutProxy (const IShortcutProxy*);
	void SetShortcut (int, const QKeySequence&);
	QMap<int, LeechCraft::ActionInfo> GetActionInfo () const;
protected:
	virtual void keyPressEvent (QKeyEvent*);
private:
	QToolBar* SetupMenuBar ();
	void SetupActions ();
	void ScheduleShowError ();
	bool IsRepr ();
private slots:
    void showError (const QString&);
	void showError ();
    void on_ActionAddFeed__triggered ();
    void on_ActionRemoveFeed__triggered ();
    void on_ActionMarkChannelAsRead__triggered ();
    void on_ActionMarkChannelAsUnread__triggered ();
	void on_ActionChannelSettings__triggered ();
    void on_ActionUpdateSelectedFeed__triggered ();
	void on_ActionItemBucket__triggered ();
	void on_ActionRegexpMatcher__triggered ();
	void on_ActionHideReadItems__triggered ();
	void on_ActionImportOPML__triggered ();
	void on_ActionExportOPML__triggered ();
	void on_ActionImportBinary__triggered ();
	void on_ActionExportBinary__triggered ();
	void on_MergeItems__toggled (bool);
	void on_ShowAsTape__toggled (bool);
    void currentChannelChanged ();
    void unreadNumberChanged (int);
    void trayIconActivated ();
signals:
    void downloadFinished (const QString&);
	void gotEntity (const LeechCraft::DownloadEntity&);
	void delegateEntity (const LeechCraft::DownloadEntity&,
			int*, QObject**);
	void bringToFront ();
};

#endif

