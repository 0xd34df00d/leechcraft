#include "player.h"
#include <QToolBar>
#include <QStatusBar>
#include <QSlider>
#include <QMessageBox>
#include <Phonon>
#include "keyinterceptor.h"
#include "videosettings.h"
#include "core.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft::Plugins::LMP;

Player::Player (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);

	dynamic_cast<QBoxLayout*> (layout ())->
		insertWidget (0, SetupToolbar ());
	StatusBar_ = new QStatusBar (this);
	layout ()->addWidget (StatusBar_);

	Core::Instance ().SetVideoWidget (Ui_.VideoWidget_);
	Core::Instance ().Reinitialize ();
	connect (Core::Instance ().GetMediaObject (),
			SIGNAL (hasVideoChanged (bool)),
			Ui_.VideoWidget_,
			SLOT (setVisible (bool)));

	ApplyVideoSettings (XmlSettingsManager::Instance ()->
				Property ("Brightness", 0).value<qreal> (),
			XmlSettingsManager::Instance ()->Property ("Contrast", 0).value<qreal> (),
			XmlSettingsManager::Instance ()->Property ("Hue", 0).value<qreal> (),
			XmlSettingsManager::Instance ()->Property ("Saturation", 0).value <qreal> ());

	KeyInterceptor *ki = new KeyInterceptor (this);
	QList<QWidget*> children = findChildren<QWidget*> ();
	for (QList<QWidget*>::iterator i = children.begin (),
			end = children.end (); i != end; ++i)
		(*i)->installEventFilter (ki);

	connect (&Core::Instance (),
			SIGNAL (stateUpdated (const QString&)),
			this,
			SLOT (handleStateUpdated (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (error (const QString&)),
			this,
			SLOT (handleError (const QString&)));
}

QToolBar* Player::SetupToolbar ()
{
	QToolBar *bar = new QToolBar;

	Play_.reset (new QAction (tr ("Play"),
				this));
	Play_->setObjectName ("Play_");
	Play_->setProperty ("ActionIcon", "lmp_play");
	connect (Play_.get (),
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (play ()));

	Pause_.reset (new QAction (tr ("Pause"),
				this));
	Pause_->setObjectName ("Pause_");
	Pause_->setProperty ("ActionIcon", "lmp_pause");
	connect (Pause_.get (),
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (pause ()));

	ViewerSettings_.reset (new QAction (tr ("Viewer settings"),
				this));
	ViewerSettings_->setObjectName ("ViewerSettings_");
	ViewerSettings_->setProperty ("ActionIcon", "lmp_viewersettings");
	connect (ViewerSettings_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (changeViewerSettings ()));

	Phonon::VolumeSlider *volumeSlider = new Phonon::VolumeSlider (this);
	Phonon::SeekSlider *seekSlider = new Phonon::SeekSlider (this);
	Core::Instance ().SetSeekSlider (seekSlider);
	Core::Instance ().SetVolumeSlider (volumeSlider);

	bar->addAction (Play_.get ());
	bar->addAction (Pause_.get ());
	bar->addSeparator ();
	bar->addAction (ViewerSettings_.get ());
	bar->addSeparator ();
	bar->addWidget (volumeSlider);
	bar->addSeparator ();
	bar->addWidget (seekSlider);

	return bar;
}

void Player::ApplyVideoSettings (qreal b, qreal c, qreal h, qreal s)
{
	Ui_.VideoWidget_->setBrightness (b);
	Ui_.VideoWidget_->setContrast (c);
	Ui_.VideoWidget_->setHue (h);
	Ui_.VideoWidget_->setSaturation (s);
}

void Player::handleStateUpdated (const QString& state)
{
	StatusBar_->showMessage (state);
}

void Player::handleError (const QString& error)
{
	QMessageBox::warning (this,
			tr ("Error"),
			error);
}

void Player::changeViewerSettings ()
{
	std::auto_ptr<VideoSettings> settings (new VideoSettings (Ui_.VideoWidget_->brightness (),
			Ui_.VideoWidget_->contrast (),
			Ui_.VideoWidget_->hue (),
			Ui_.VideoWidget_->saturation (),
			this));
	if (settings->exec () == QDialog::Rejected)
		return;

	qreal b = settings->Brightness (),
		  c = settings->Contrast (),
		  h = settings->Hue (),
		  s = settings->Saturation ();

	ApplyVideoSettings (b, c, h, s);

	XmlSettingsManager::Instance ()->setProperty ("Brightness", b);
	XmlSettingsManager::Instance ()->setProperty ("Contrast", c);
	XmlSettingsManager::Instance ()->setProperty ("Hue", h);
	XmlSettingsManager::Instance ()->setProperty ("Saturation", s);
}

