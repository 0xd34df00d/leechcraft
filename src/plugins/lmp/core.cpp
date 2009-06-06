#include "core.h"
#include <QUrl>
#include <QTextCodec>
#include <QMainWindow>
#include "xmlsettingsmanager.h"

using namespace LeechCraft::Plugins::LMP;
using namespace Phonon;

Core::Core ()
: TotalTimeAvailable_ (false)
, VideoWidget_ (0)
, ShowAction_ (new QAction (QIcon (":/plugins/lmp/resources/images/lmp.png"),
			tr ("Show LMP"), this))
{
	ShowAction_->setEnabled (false);
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
	Player_.reset ();
}

void Core::SetCoreProxy (ICoreProxy_ptr proxy)
{
	Proxy_ = proxy;
}

ICoreProxy_ptr Core::GetCoreProxy () const
{
	return Proxy_;
}

void Core::Reinitialize ()
{
	TotalTimeAvailable_ = false;

	MediaObject_.reset (new MediaObject (this));
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
	AudioOutput_.reset (new AudioOutput (MusicCategory, this));
	AudioOutput_->setVolume (oldVolume);

	SeekSlider_->setMediaObject (MediaObject_.get ());
	VolumeSlider_->setAudioOutput (AudioOutput_.get ());
}

MediaObject* Core::GetMediaObject () const
{
	return MediaObject_.get ();
}

void Core::SetVideoWidget (VideoWidget *widget)
{
	VideoWidget_ = widget;
}

void Core::SetSeekSlider (SeekSlider *slider)
{
	SeekSlider_ = slider;
}

void Core::SetVolumeSlider (VolumeSlider *slider)
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
		if (MediaObject_->state () == PausedState)
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

QAction* Core::GetShowAction () const
{
	return ShowAction_;
}

void Core::Handle (const LeechCraft::DownloadEntity& e)
{
	QString source = QTextCodec::codecForName ("UTF-8")->
			toUnicode (e.Entity_);
	if (!Player_.get ())
	{
		Player_.reset (new Player (Proxy_->GetMainWindow ()));
		ShowAction_->setEnabled (true);
		connect (ShowAction_,
				SIGNAL (triggered ()),
				Player_.get (),
				SLOT (show ()));
	}
	Player_->show ();
	Player_->Enqueue (new MediaSource (source));
	if (!MediaObject_->queue ().size ())
	{
		play ();
		VideoWidget_->setVisible (MediaObject_->hasVideo ());
	}
}

void Core::play ()
{
	if (MediaObject_.get ())
	{
		if (!VideoPath_.isValid ())
			VideoPath_.reconnect (MediaObject_.get (), VideoWidget_);
		if (!AudioPath_.isValid ())
			AudioPath_.reconnect (MediaObject_.get (), AudioOutput_.get ());

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
		case LoadingState:
			result = tr ("Initializing");
			break;
		case StoppedState:
			result = tr ("Stopped");
			break;
		case PlayingState:
			result = tr ("Playing");
			break;
		case BufferingState:
			result = tr ("Buffering");
			break;
		case PausedState:
			result = tr ("Paused");
			break;
		case ErrorState:
			result = tr ("Error");
			emit stateUpdated (result);
			break;
	}
	if (MediaObject_->state () == ErrorState)
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

	result += tr (" from ");
	MediaSource source = MediaObject_->currentSource ();
	switch (source.type ())
	{
		case MediaSource::Invalid:
#if PHONON_VERSION >= PHONON_VERSION_CHECK (4, 3, 0)
		case MediaSource::Empty:
#endif
			result += tr ("nowhere");
			break;
		case MediaSource::LocalFile:
			result += source.fileName ();
			break;
		case MediaSource::Url:
			result += source.url ().toString ();
			break;
		case MediaSource::Disc:
			result += source.deviceName ();
			switch (source.discType ())
			{
				case Cd:
					result += tr (" (CD)");
					break;
				case Dvd:
					result += tr (" (DVD)");
					break;
				case Vcd:
					result += tr (" (VCD)");
					break;
				default:
					result += tr (" (Unknown disc type)");
					break;
			}
			break;
		case MediaSource::Stream:
			result += tr ("stream");
			break;
	}

	if (MediaObject_->state () == ErrorState)
		emit error (result);
	else
		emit stateUpdated (result);
}

void Core::totalTimeChanged ()
{
	TotalTimeAvailable_ = true;
}

void Core::handleHasVideoChanged (bool)
{
}

