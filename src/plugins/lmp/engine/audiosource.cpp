/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audiosource.h"
#include <QUrl>

namespace LC
{
namespace LMP
{
	AudioSource::AudioSource ()
	{
	}

	AudioSource::AudioSource (const QString& filename)
	: Url_ (QUrl::fromLocalFile (filename))
	{
	}

	AudioSource::AudioSource (const QUrl& url)
	: Url_ (url)
	{
	}

	AudioSource::AudioSource (const AudioSource& source)
	: Url_ (source.Url_)
	{
	}

	AudioSource& AudioSource::operator= (const AudioSource& source)
	{
		Url_ = source.Url_;
		return *this;
	}

	bool AudioSource::operator== (const AudioSource& source) const
	{
		return Url_ == source.Url_;
	}

	bool AudioSource::operator!= (const AudioSource& source) const
	{
		return !(*this == source);
	}

	QUrl AudioSource::ToUrl () const
	{
		return Url_;
	}

	bool AudioSource::IsLocalFile () const
	{
		return Url_.scheme () == "file";
	}

	QString AudioSource::GetLocalPath () const
	{
		return Url_.toLocalFile ();
	}

	bool AudioSource::IsRemote () const
	{
		if (IsEmpty ())
			return false;

		return !IsLocalFile ();
	}

	bool AudioSource::IsEmpty () const
	{
		return !Url_.isValid ();
	}

	void AudioSource::Clear ()
	{
		Url_.clear ();
	}

	AudioSource::Type AudioSource::GetType () const
	{
		return IsLocalFile () ? Type::File : Type::Url;
	}

	uint qHash (const AudioSource& source)
	{
		return qHash (source.ToUrl ());
	}
}
}
