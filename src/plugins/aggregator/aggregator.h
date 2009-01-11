#ifndef AGGREGATOR_H
#define AGGREGATOR_H
#include <memory>
#include <interfaces/interfaces.h>
#include <QWidget>
#include <QItemSelection>

class QSystemTrayIcon;
class QTranslator;
class QToolBar;
class ItemBucket;

struct Aggregator_Impl;

class Aggregator : public QWidget
                 , public IInfo
                 , public IEmbedTab
				 , public IHaveSettings
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab IHaveSettings)

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
protected:
	virtual void keyPressEvent (QKeyEvent*);
private:
	void SetupMenuBar ();
private slots:
    void showError (const QString&);
    void on_ActionAddFeed__triggered ();
    void on_ActionRemoveFeed__triggered ();
    void on_ActionMarkItemAsUnread__triggered ();
    void on_ActionMarkChannelAsRead__triggered ();
    void on_ActionMarkChannelAsUnread__triggered ();
    void on_ActionUpdateSelectedFeed__triggered ();
    void on_ChannelTags__editingFinished ();
    void on_CaseSensitiveSearch__stateChanged (int);
	void on_ActionAddToItemBucket__triggered ();
	void on_ActionItemBucket__triggered ();
	void on_ActionRegexpMatcher__triggered ();
	void on_ActionHideReadItems__triggered ();
	void on_ActionImportOPML__triggered ();
	void on_ActionExportOPML__triggered ();
	void on_ActionImportBinary__triggered ();
	void on_ActionExportBinary__triggered ();
	void on_ItemCommentsSubscribe__released ();
	void on_ItemCategoriesButton__released ();
    void currentItemChanged (const QItemSelection&);
    void currentChannelChanged ();
    void unreadNumberChanged (int);
    void trayIconActivated ();
	void updateItemsFilter ();
	void updatePixmap (int);
	void viewerSettingsChanged ();
	void makeCurrentItemVisible ();
signals:
    void downloadFinished (const QString&);
	void fileDownloaded (const QString&);
	void bringToFront ();
};

#endif

