#ifndef AGGREGATOR_H
#define AGGREGATOR_H
#include <interfaces/interfaces.h>
#include "ui_mainwindow.h"

class XmlSettingsDialog;
class QSortFilterProxyModel;
class ChannelsFilterModel;
class TagsCompleter;

class Aggregator : public QMainWindow
                 , public IInfo
                 , public IWindow
{
    Q_OBJECT
    Q_INTERFACES (IInfo IWindow)

    Ui::MainWindow Ui_;
    unsigned long int ID_;
    bool IsShown_;

    QMenu *Plugins_;
    XmlSettingsDialog *XmlSettingsDialog_;
    QSortFilterProxyModel *ItemsFilterModel_;
    ChannelsFilterModel *ChannelsFilterModel_;
    TagsCompleter *TagsCompleter_;
public:
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
    void SetParent (QWidget*);
    void ShowWindow ();
    void ShowBalloonTip ();
protected:
    virtual void closeEvent (QCloseEvent*);
public slots:
    void handleHidePlugins ();
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
    void currentItemChanged (const QModelIndex&);
    void currentChannelChanged ();
signals:
    void downloadFinished (const QString&);
};

#endif

