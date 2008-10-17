#include "fancypopupmanager.h"
#include <numeric>
#include <algorithm>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QtDebug>
#include "fancypopup.h"
#include "xmlsettingsmanager.h"

Main::FancyPopupManager::FancyPopupManager (QObject *parent)
: QObject (parent)
{
	QTimer *timer = new QTimer (this);
	connect (timer,
			SIGNAL (timeout ()),
			this,
			SLOT (timerTimeout ()));

	timer->start (500);
}

Main::FancyPopupManager::~FancyPopupManager ()
{
	for (popups_t::iterator i = Popups_.begin (), end = Popups_.end ();
			i != end; ++i)
		delete *i;
}

namespace
{
	struct HeightRetriever
	{
		Main::FancyPopup *Popup_;

		HeightRetriever (Main::FancyPopup *p)
		: Popup_ (p)
		{
		}

		int operator() (int previous, Main::FancyPopup *popup)
		{
			return popup == Popup_ ? 0 : previous + popup->height ();
		}
	};
};

void Main::FancyPopupManager::ShowMessage (const QString& title,
		const QString& message)
{
	FancyPopup *popup = new FancyPopup (title, message);
	connect (popup,
			SIGNAL (clicked ()),
			this,
			SLOT (popupClicked ()));

	popup->move (CalculatePosition (popup));

	Popups_.push_back (popup);
	Dates_ [QDateTime::currentDateTime ()] = popup;
	popup->show ();
}

void Main::FancyPopupManager::timerTimeout ()
{
	QDateTime current = QDateTime::currentDateTime ();

	for (dates_t::iterator i = Dates_.begin ();
			i != Dates_.end (); ++i)
		if (i->first.secsTo (current) >=
				XmlSettingsManager::Instance ()->
				property ("FinishedDownloadMessageTimeout").toInt ())
		{
			FancyPopup *popUp = i->second;

			popups_t::iterator position = std::find (Popups_.begin (),
					Popups_.end (), popUp);

			popUp->hide ();

			Popups_.erase (position);
			Dates_.erase (i++);
			delete popUp;
			popUp = 0;
		}

	RecalculatePositions ();
}

void Main::FancyPopupManager::popupClicked ()
{
	FancyPopup *popup = qobject_cast<FancyPopup*> (sender ());
	if (!popup)
		return;

	popups_t::iterator position = std::find (Popups_.begin (),
			Popups_.end (), popup);
	if (position == Popups_.end ())
		return;

	Popups_.erase (position);

	Dates_.erase (std::find_if (Dates_.begin (),
			Dates_.end (), ValueFinder (popup)));

	delete popup;
	popup = 0;

	RecalculatePositions ();
}

void Main::FancyPopupManager::RecalculatePositions ()
{
	for (popups_t::iterator i = Popups_.begin (),
			end = Popups_.end (); i != end; ++i)
		(*i)->move (CalculatePosition (*i));
}

QPoint Main::FancyPopupManager::CalculatePosition (FancyPopup *popup)
{
	int sumHeight = std::accumulate (Popups_.begin (),
			std::find (Popups_.begin (), Popups_.end (),
				popup), 0, HeightRetriever (popup));

	return CalculatePosition (popup, sumHeight);
}

QPoint Main::FancyPopupManager::CalculatePosition (FancyPopup *popup, int height)
{
	QRect geom = qApp->desktop ()->availableGeometry (popup);
	QPoint dest (geom.x () + geom.width (), geom.y () + geom.height ());

	int layout = 1;
	if (dest.y () > qApp->desktop ()->screenGeometry ().height () / 2)
		layout = -1;

	if (dest.x () + popup->width () > geom.x () + geom.width ())
		dest.setX (geom.x () + geom.width () - popup->width ());

	if (dest.x () < 0)
		dest.setX (0);

	if (dest.y () + popup->height () > geom.y () + geom.height ())
		dest.setY (geom.y () + geom.height () - popup->height ());

	if (dest.y () < 0)
		dest.setY (0);


	dest.setY (dest.y () + layout * height);

	return dest;
}

