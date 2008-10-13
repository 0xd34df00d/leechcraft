#ifndef AGGREGATOR_H
#define AGGREGATOR_H
#include <memory>
#include <interfaces/interfaces.h>
#include "ui_mainwidget.h"

class XmlSettingsDialog;
class ItemsFilterModel;
class ChannelsFilterModel;
class TagsCompleter;
class QSystemTrayIcon;
class QTranslator;
class QToolBar;
class ItemBucket;

class Aggregator : public QWidget
                 , public IInfo
                 , public IEmbedTab
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab)

    Ui::MainWidget Ui_;
    unsigned long int ID_;

    QMenu *Plugins_;
	QToolBar *ToolBar_;
    QAction *ActionAddFeed_;
    QAction *ActionPreferences_;
    QAction *ActionUpdateFeeds_;
    QAction *ActionRemoveFeed_;
    QAction *ActionMarkItemAsUnread_;
    QAction *ActionMarkChannelAsRead_;
    QAction *ActionMarkChannelAsUnread_;
    QAction *ActionUpdateSelectedFeed_;
    QAction *ActionAddToItemBucket_;
    QAction *ActionItemBucket_;
    QAction *ActionRegexpMatcher_;
    QAction *ActionHideReadItems_;
    QAction *ActionImportOPML_;
    QAction *ActionExportOPML_;

	std::auto_ptr<XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<ItemsFilterModel> ItemsFilterModel_;
	std::auto_ptr<ChannelsFilterModel> ChannelsFilterModel_;
	std::auto_ptr<TagsCompleter> TagsLineCompleter_, ChannelTagsCompleter_;
	std::auto_ptr<QSystemTrayIcon> TrayIcon_;
	std::auto_ptr<QTranslator> Translator_;
    std::auto_ptr<ItemBucket> ItemBucket_;
public:
	virtual ~Aggregator ();
    void Init ();
    void Release ();
    QString GetName () const;
    QString GetInfo () const;
    QString GetStatusbarMessage () const;
    IInfo& SetID (long unsigned int);
    unsigned long int GetID () const;
    QStringList Provides () const;
    QStringList Needs () const;
    QStringList Uses () const;
    void SetProvider (QObject*, const QString&);
    void PushMainWindowExternals (const MainWindowExternals&);
    QIcon GetIcon () const;
	QWidget* GetTabContents ();
private:
	void SetupMenuBar ();
private slots:
    void showError (const QString&);
    void on_ActionAddFeed__triggered ();
    void on_ActionRemoveFeed__triggered ();
    void on_ActionPreferences__triggered ();
    void on_Items__activated (const QModelIndex&);
    void on_Feeds__activated (const QModelIndex&);
    void on_ActionMarkItemAsUnread__triggered ();
    void on_ActionMarkChannelAsRead__triggered ();
    void on_ActionMarkChannelAsUnread__triggered ();
    void on_ActionUpdateSelectedFeed__triggered ();
    void on_ChannelTags__textChanged (const QString&);
    void on_ChannelTags__editingFinished ();
    void on_CaseSensitiveSearch__stateChanged (int);
	void on_ActionAddToItemBucket__triggered ();
	void on_ActionItemBucket__triggered ();
	void on_ActionRegexpMatcher__triggered ();
	void on_ActionHideReadItems__triggered ();
	void on_ActionImportOPML__triggered ();
	void on_ActionExportOPML__triggered ();
    void currentItemChanged (const QModelIndex&);
    void currentChannelChanged ();
    void unreadNumberChanged (int);
    void trayIconActivated ();
	void updateItemsFilter ();
	void updatePixmap (int);
	void viewerSettingsChanged ();
signals:
    void downloadFinished (const QString&);
	void fileDownloaded (const QString&);
	void bringToFront ();
};

#endif

