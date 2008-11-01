#include "core.h"
#include <QUrl>

Core::Core ()
: TotalTimeAvailable_ (false)
{
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	MediaObject_.reset ();
}

void Core::Reinitialize (const QString& entity)
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

	MediaObject_->setCurrentSource (entity);

	AudioOutput_.reset (new Phonon::AudioOutput (Phonon::MusicCategory, this));
}

Phonon::MediaObject* Core::GetMediaObject () const
{
	return MediaObject_.get ();
}

Phonon::AudioOutput* Core::GetAudioOutput () const
{
	return AudioOutput_.get ();
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

