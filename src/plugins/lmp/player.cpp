#include "player.h"
#include <QToolBar>
#include <QStatusBar>
#include <QSlider>
#include <QMessageBox>
#include <QStandardItem>
#include <QUrl>
#include "keyinterceptor.h"
#include "videosettings.h"
#include "core.h"
#include "xmlsettingsmanager.h"

Q_DECLARE_METATYPE (Phonon::MediaSource*);

using namespace LeechCraft::Plugins::LMP;
using namespace Phonon;

Player::Player (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);

	dynamic_cast<QBoxLayout*> (layout ())->
		insertWidget (0, SetupToolbar ());
	StatusBar_ = new QStatusBar (this);
	layout ()->addWidget (StatusBar_);

	QueueModel_.reset (new QStandardItemModel);
	QueueModel_->setHorizontalHeaderLabels (QStringList (tr ("Media source"))
			<< tr ("Source type"));
	Ui_.Queue_->setModel (QueueModel_.get ());

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

void Player::Enqueue (MediaSource *source)
{
	QList<QStandardItem*> items;
	switch (source->type ())
	{
		case MediaSource::LocalFile:
			items << new QStandardItem (source->fileName ())
				<< new QStandardItem (tr ("File"));
			break;
		case MediaSource::Url:
			items << new QStandardItem (source->url ().toString ())
				<< new QStandardItem (tr ("URL"));
			break;
		case MediaSource::Disc:
			items << new QStandardItem (source->deviceName ());
			switch (source->discType ())
			{
				case Cd:
					items << new QStandardItem (tr ("Audio CD"));
					break;
				case Dvd:
					items << new QStandardItem (tr ("DVD"));
					break;
				case Vcd:
					items << new QStandardItem (tr ("Video CD"));
					break;
				default:
					items << new QStandardItem (tr ("Unknown disc type"));
					break;
			}
			break;
		case MediaSource::Stream:
			items << new QStandardItem (source->fileName ())
				<< new QStandardItem (tr ("Stream"));
			break;
		case MediaSource::Invalid:
		case MediaSource::Empty:
			return;
	}

	items.at (0)->setData (QVariant::fromValue<MediaSource*> (source), SourceRole);

	QueueModel_->appendRow (items);
	Core::Instance ().GetMediaObject ()->enqueue (*source);
}

QToolBar* Player::SetupToolbar ()
{
	QToolBar *bar = new QToolBar (this);

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

	VolumeSlider *volumeSlider = new VolumeSlider (this);
	SeekSlider *seekSlider = new SeekSlider (this);
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

void Player::on_Queue__activated (const QModelIndex& si)
{
	MediaObject *o = Core::Instance ().GetMediaObject ();

	MediaSource *source = QueueModel_->item (si.row ())->data (SourceRole).value<MediaSource*> ();
	o->clearQueue ();
	o->setCurrentSource (*source);
	o->play ();
	for (int i = si.row (); i < QueueModel_->rowCount (); ++i)
		o->enqueue (*QueueModel_->item (i)->data (SourceRole).value<MediaSource*> ());
}

