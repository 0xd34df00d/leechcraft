#ifndef AGGREGATOR_H
#define AGGREGATOR_H
#include <memory>
#include <interfaces/iinfo.h>
#include <interfaces/iembedtab.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavesettings.h>
#include <QWidget>
#include <QItemSelection>

class QSystemTrayIcon;
class QTranslator;
class QToolBar;
class ItemBucket;
class Enclosure;

struct Aggregator_Impl;

class Aggregator : public QWidget
                 , public IInfo
                 , public IEmbedTab
				 , public IHaveSettings
				 , public IJobHolder
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab IHaveSettings IJobHolder)

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
	LeechCraft::Util::XmlSettingsDialog* GetSettingsDialog () const;
	QAbstractItemModel* GetRepresentation () const;
	LeechCraft::Util::HistoryModel* GetHistory () const;
	QWidget* GetControls () const;
	QWidget* GetAdditionalInfo () const;
	void ItemSelected (const QModelIndex&);
protected:
	virtual void keyPressEvent (QKeyEvent*);
private:
	QToolBar* SetupMenuBar ();
	void SetupActions ();
private slots:
    void showError (const QString&);
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
    void currentChannelChanged ();
    void unreadNumberChanged (int);
    void trayIconActivated ();
signals:
    void downloadFinished (const QString&);
	void gotEntity (const QByteArray&);
	void bringToFront ();
};

#endif

