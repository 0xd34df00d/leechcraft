#include "twitterpage.h"
#include "core.h"
#include <qjson/parser.h>


Q_DECLARE_METATYPE (QObject**);

namespace LeechCraft
{
namespace Woodpecker
{

QObject *TwitterPage::S_MultiTabsParent_ = 0;

TwitterPage::TwitterPage (QWidget *parent) : QWidget (parent),
	ui (new Ui::TwitterPage),
	Toolbar_ (new QToolBar)
{
	ui->setupUi (this);
//	Toolbar_->addAction(ui->actionRefresh);
	interface = new twitterInterface (this);
	connect (interface, SIGNAL (tweetsReady (QList<std::shared_ptr<Tweet> >)),
			 this, SLOT (updateTweetList (QList<std::shared_ptr<Tweet> >)));
	timer = new QTimer (this);
	timer->setInterval (90e3); // Update twits every 1.5 minutes
	connect (timer, SIGNAL (timeout()), interface, SLOT (getHomeFeed()));
	tryToLogin();
	int newSliderPos;

	connect((ui->TwitList_->verticalScrollBar()), SIGNAL (valueChanged(int)),
			this, SLOT (scrolledDown(int)));
//    connect(ui->login_Test_, SIGNAL (clicked ()), SLOT(tryToLogin()));
	connect (ui->TwitButton_, SIGNAL (clicked()), SLOT (twit()));
	settings = new QSettings (QCoreApplication::organizationName (),
							  QCoreApplication::applicationName () + "_Woodpecker");
	connect(ui->TwitList_, SIGNAL(clicked()), SLOT(getHomeFeed()));

	if ( (! settings->value ("token").isNull()) && (! settings->value ("tokenSecret").isNull()))
	{
		qDebug() << "Have an authorized" << settings->value ("token") << ":" << settings->value ("tokenSecret");
		interface->login (settings->value ("token").toString(), settings->value ("tokenSecret").toString());
		interface->getHomeFeed();
		timer->start();
	}

}

TwitterPage::~TwitterPage()
{
	settings->deleteLater();
	timer->stop();
	timer->deleteLater();
	delete interface;
	delete ui;
}

TabClassInfo TwitterPage::GetTabClassInfo () const
{
	return Core::Instance ().GetTabClass ();
}

QToolBar* TwitterPage::GetToolBar () const
{
	return Toolbar_;
}

QObject* TwitterPage::ParentMultiTabs ()
{
	return S_MultiTabsParent_;
}

QList<QAction*> TwitterPage::GetTabBarContextMenuActions () const
{
	return QList<QAction*> ();
}

QMap<QString, QList<QAction*> > TwitterPage::GetWindowMenus () const
{
	return WindowMenus_;
}

void TwitterPage::SetParentMultiTabs (QObject *parent)
{
	S_MultiTabsParent_ = parent;
}

void TwitterPage::Remove()
{
	emit removeTab (this);
	deleteLater ();
}

void TwitterPage::tryToLogin()
{
	interface->getAccess();
	connect (interface, SIGNAL (authorized (QString, QString)), SLOT (recvdAuth (QString, QString)));
}

void TwitterPage::requestUserTimeline (QString username)
{
	interface->getUserTimeline (username);
}

void TwitterPage::updateTweetList (QList< std::shared_ptr< Tweet > > twits)
{
	std::shared_ptr<Tweet> twit;
	std::shared_ptr<Tweet> firstNewTwit;
	int i;

	if (! (twits.length())) return; // if we have no tweets to parse

	firstNewTwit = twits.first();

	if (screenTwits.length() && (twits.last()->id() == screenTwits.first()->id())) // if we should prepend
		for (auto i = twits.end()-2; i >= twits.begin(); i--)
			screenTwits.insert(0,*i);
	else
	{

		// Now we'd find firstNewTwit in twitList

		for (i = 0; i < screenTwits.length(); i++)
			if ( (screenTwits.at (i)->id()) == firstNewTwit->id()) break;

		int insertionShift = screenTwits.length() - i;    // We've already got insertionShift twits to our list


		for (i = 0; i < insertionShift; i++)
		{
			qDebug() << "uTL: cycle i " << i << endl;
			twits.removeFirst();
		}


		screenTwits.append (twits);
	}
	ui->TwitList_->clear();

	Q_FOREACH (twit, screenTwits)
	{
		QListWidgetItem *tmpitem = new QListWidgetItem();
		QIcon icon (twit->author()->avatar);
		tmpitem->setText (twit->text() + "\n" +
						  "\t\t" + twit->author()->username() + "\t" +
						  twit->dateTime().toLocalTime().toString());
		tmpitem->setIcon (icon);
		ui->TwitList_->insertItem (0, tmpitem);
	}
	ui->TwitList_->update();
	ui->TwitList_->setEnabled(true);
}

void TwitterPage::recvdAuth (QString token, QString tokenSecret)
{
	settings->setValue ("token", token);
	settings->setValue ("tokenSecret", tokenSecret);
	interface->getHomeFeed();
	timer->start();
}

void TwitterPage::twit()
{
	interface->sendTweet (ui->TwitEdit_->text());
	ui->TwitEdit_->clear();
}

void TwitterPage::scrolledDown (int sliderPos)
{
	qDebug() << "Scrolled down to: " << sliderPos << " Min/Max: " <<
		ui->TwitList_->verticalScrollBar()->minimum() << "/" << ui->TwitList_->verticalScrollBar()->maximum();
	if (sliderPos == ui->TwitList_->verticalScrollBar()->maximum())
	{
		ui->TwitList_->verticalScrollBar()->setSliderPosition(ui->TwitList_->verticalScrollBar()->maximum()-1);

		ui->TwitList_->setEnabled(false);
		interface->getMoreTweets(QString("%1").arg((*(screenTwits.begin()))->id()));
	}
}


}
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
