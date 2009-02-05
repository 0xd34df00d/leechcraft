#include "fancypopupmanager.h"
#include <numeric>
#include <algorithm>
#include <iterator>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QSystemTrayIcon>
#include <QtDebug>
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

LeechCraft::FancyPopupManager::FancyPopupManager (QSystemTrayIcon *icon,
		QObject *parent)
: QObject (parent)
, TrayIcon_ (icon)
{
	QTimer *timer = new QTimer (this);
	connect (timer,
			SIGNAL (timeout ()),
			this,
			SLOT (timerTimeout ()));

	timer->start (500);

	connect (TrayIcon_,
			SIGNAL (messageClicked ()),
			this,
			SLOT (handleMessageClicked ()));
}

LeechCraft::FancyPopupManager::~FancyPopupManager ()
{
}

void LeechCraft::FancyPopupManager::ShowMessage (const QString& message)
{
	Popups_.push_back (message);
	Dates_ [QDateTime::currentDateTime ()] = message;

	UpdateMessage ();
}

void LeechCraft::FancyPopupManager::timerTimeout ()
{
	QDateTime current = QDateTime::currentDateTime ();

	for (dates_t::iterator i = Dates_.begin ();
			i != Dates_.end (); ++i)
		if (i->first.secsTo (current) >=
				XmlSettingsManager::Instance ()->
				property ("FinishedDownloadMessageTimeout").toInt ())
		{
			Popups_.erase (std::find (Popups_.begin (),
					Popups_.end (), i->second));
			Dates_.erase (i);

			UpdateMessage ();
			break;
		}
}

void LeechCraft::FancyPopupManager::handleMessageClicked ()
{
	Dates_.clear ();
	Popups_.clear ();
}

void LeechCraft::FancyPopupManager::UpdateMessage ()
{
	QString message;
	for (popups_t::const_iterator i = Popups_.begin (),
			begin = Popups_.begin (),
			end = Popups_.end (); i != end; ++i)
	{
		message += *i;
		message += "\r\n";
		if (std::distance (begin, i) >= 12)
			break;
	}

	if (!message.isEmpty ())
		TrayIcon_->showMessage (tr ("LeechCraft Notification"),
				message,
				QSystemTrayIcon::Information,
				XmlSettingsManager::Instance ()->
				property ("FinishedDownloadMessageTimeout").toInt () * 1000);
}

