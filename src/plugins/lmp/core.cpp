#include "core.h"
#include <QUrl>
#include <videowidget.h>
#include <seekslider.h>
#include <volumeslider.h>
#include "xmlsettingsmanager.h"

Core::Core ()
: TotalTimeAvailable_ (false)
, VideoWidget_ (0)
{
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	if (AudioOutput_.get ())
		XmlSettingsManager::Instance ()->setProperty ("Volume", AudioOutput_->volume ());
	MediaObject_.reset ();
}

void Core::Reinitialize ()
{
	TotalTimeAvailable_ = false;

	MediaObject_.reset (new Phonon::MediaObject (this));
	MediaObject_->setTickInterval (100);
	connect (MediaObject_.get (),
			SIGNAL (totalTimeChanged (qint64)),
			this,
			SLOT (totalTimeChanged ()));
	connect (MediaObject_.get (),
			SIGNAL (tick (qint64)),
			this,
			SLOT (updateState ()));
	connect (MediaObject_.get (),
			SIGNAL (stateChanged (Phonon::State,
					Phonon::State)),
			this,
			SLOT (updateState ()));
	connect (MediaObject_.get (),
			SIGNAL (hasVideoChanged (bool)),
			this,
			SLOT (handleHasVideoChanged (bool)));

	qreal oldVolume = XmlSettingsManager::Instance ()->Property ("Volume", 1).value<qreal> ();
	if (AudioOutput_.get ())
		oldVolume = AudioOutput_->volume ();
	AudioOutput_.reset (new Phonon::AudioOutput (Phonon::MusicCategory, this));
	AudioOutput_->setVolume (oldVolume);

	Phonon::createPath (Core::Instance ().GetMediaObject (),
			VideoWidget_);
	Phonon::createPath (Core::Instance ().GetMediaObject (),
			AudioOutput_.get ());

	SeekSlider_->setMediaObject (MediaObject_.get ());
	VolumeSlider_->setAudioOutput (AudioOutput_.get ());
}

Phonon::MediaObject* Core::GetMediaObject () const
{
	return MediaObject_.get ();
}

void Core::SetVideoWidget (Phonon::VideoWidget *widget)
{
	VideoWidget_ = widget;
}

void Core::SetSeekSlider (Phonon::SeekSlider *slider)
{
	SeekSlider_ = slider;
}

void Core::SetVolumeSlider (Phonon::VolumeSlider *slider)
{
	VolumeSlider_ = slider;
}

void Core::IncrementVolume ()
{
	qreal nv = AudioOutput_->volume ();
	nv += 0.1;
	if (nv > 1)
		nv = 1;
	AudioOutput_->setVolume (nv);
}

void Core::DecrementVolume ()
{
	qreal nv = AudioOutput_->volume ();
	nv -= 0.1;
	if (nv < 0)
		nv = 0;
	AudioOutput_->setVolume (nv);
}

void Core::ToggleFullScreen ()
{
	VideoWidget_->setFullScreen (1 - VideoWidget_->isFullScreen ());
}

void Core::TogglePause ()
{
	if (MediaObject_.get ())
	{
		if (MediaObject_->state () == Phonon::PausedState)
			play ();
		else
			pause ();
	}
}

void Core::Forward (SkipAmount a)
{
	if (MediaObject_.get ())
		MediaObject_->seek (MediaObject_->currentTime () + a * 1000);
}

void Core::Rewind (SkipAmount a)
{
	if (MediaObject_.get ())
		MediaObject_->seek (MediaObject_->currentTime () - a * 1000);
}

void Core::play ()
{
	if (MediaObject_.get ())
	{
		MediaObject_->play ();
		emit bringToFront ();
	}
}

void Core::pause ()
{
	if (MediaObject_.get ())
		MediaObject_->pause ();
}

void Core::setSource (const QString& filename)
{
	MediaObject_->setCurrentSource (filename);
}

void Core::updateState ()
{
	QString result;
	switch (MediaObject_->state ())
	{
		case Phonon::LoadingState:
			result = tr ("Initializing");
			break;
		case Phonon::StoppedState:
			result = tr ("Stopped");
			break;
		case Phonon::PlayingState:
			result = tr ("Playing");
			break;
		case Phonon::BufferingState:
			result = tr ("Buffering");
			break;
		case Phonon::PausedState:
			result = tr ("Paused");
			break;
		case Phonon::ErrorState:
			result = tr ("Error");
			break;
	}
	if (MediaObject_->state () == Phonon::ErrorState)
		result += tr (" (%1)").arg (MediaObject_->errorString ()); 
	result += tr (" [");
	result += QString::number (static_cast<double> (MediaObject_->
				currentTime ())/1000., 'f', 1);
	if (TotalTimeAvailable_)
	{
		result += tr ("/");
		result += QString::number (static_cast<double> (MediaObject_->
					totalTime ())/1000., 'f', 1);
	}
	result += tr ("]");

	result += tr (" borrowing data from ");
	Phonon::MediaSource source = MediaObject_->currentSource ();
	switch (source.type ())
	{
		case Phonon::MediaSource::Invalid:
			result += tr ("nowhere");
			break;
		case Phonon::MediaSource::LocalFile:
			result += source.fileName ();
			break;
		case Phonon::MediaSource::Url:
			result += source.url ().toString ();
			break;
		case Phonon::MediaSource::Disc:
			result += source.deviceName ();
			switch (source.discType ())
			{
				case Phonon::Cd:
					result += tr (" (CD)");
					break;
				case Phonon::Dvd:
					result += tr (" (DVD)");
					break;
				case Phonon::Vcd:
					result += tr (" (VCD)");
					break;
				default:
					result += tr (" (Unknown disc type)");
					break;
			}
			break;
		case Phonon::MediaSource::Stream:
			result += tr ("stream");
			break;
	}

	emit stateUpdated (result);
}

void Core::totalTimeChanged ()
{
	TotalTimeAvailable_ = true;
}

void Core::handleHasVideoChanged (bool)
{
}

