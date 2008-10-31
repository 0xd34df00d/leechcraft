#include "core.h"

Core::Core ()
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
	MediaObject_.reset (new Phonon::MediaObject (this));
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

