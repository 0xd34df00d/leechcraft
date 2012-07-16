#ifndef TWITTERPAGE_H
#define TWITTERPAGE_H

#include <QWidget>
#include <QScrollBar>
#include <QTimer>
#include <QMenu>
#include <QAction>

#include <interfaces/ihavetabs.h>
#include <interfaces/structures.h>

#include "twitterinterface.h"

#include "ui_twitterpage.h"

namespace Ui {
class TwitterPage;
}

namespace LeechCraft
{
namespace Woodpecker
{
class TwitterPage : public QWidget
    , public ITabWidget
{
    Q_OBJECT
    Q_INTERFACES (ITabWidget)

//  Ui::TwitterPage Ui_;

    static QObject* S_MultiTabsParent_;
    QToolBar *Toolbar_;
    QMenu *DoctypeMenu_;
    QMenu *RecentFilesMenu_;
    QString Filename_;
    bool Modified_;
    QMap<QString, QList<QAction*> > WindowMenus_;
    QHash<QString, QString> Extension2Lang_;

    QtMsgHandler DefaultMsgHandler_;
    QObject *WrappedObject_;
    bool TemporaryDocument_;
	
	QAction *actionRetwit_;
	QAction *actionReply_;
	QAction *actionSPAM_;
public:
    explicit TwitterPage (QWidget *parent = 0);
    ~TwitterPage();
    static void SetParentMultiTabs (QObject*);


    void Remove ();
    QToolBar* GetToolBar () const;
    QObject* ParentMultiTabs ();
    QList<QAction*> GetTabBarContextMenuActions () const;
    QMap<QString, QList<QAction*> > GetWindowMenus () const;
    TabClassInfo GetTabClassInfo () const;

private slots:
	void on_TwitList__customContextMenuRequested (const QPoint&);
	
public slots:
    void tryToLogin();
    void requestUserTimeline (QString username);
    void updateTweetList (QList<std::shared_ptr<Tweet>> twits);
    void recvdAuth (QString token, QString tokenSecret);
    void twit();
    void retwit();
    void reply();
    void reply(QListWidgetItem*);
	void reportSpam();
    void sendReply();
	void scrolledDown(int sliderPos);


private:
    Ui::TwitterPage *ui;
    twitterInterface *interface;
    QTimer *timer;
    QSettings *settings;
    QList<std::shared_ptr<Tweet>> screenTwits;

signals:
    void removeTab (QWidget*);
    void changeTabName (QWidget*, const QString&);
    void changeTabIcon (QWidget*, const QIcon&);
    void changeTooltip (QWidget*, QWidget*);
    void statusBarChanged (QWidget*, const QString&);
    void couldHandle (const LeechCraft::Entity&, bool*);
    void delegateEntity (const LeechCraft::Entity&,
                         int*, QObject**);
    void gotEntity (const LeechCraft::Entity&);

};
}
}

#endif // TWITTERPAGE_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
