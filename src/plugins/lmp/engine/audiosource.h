/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QUrl>
#include <QMetaType>

namespace LC
{
namespace LMP
{
	class AudioSource
	{
		QUrl Url_;
	public:
		enum class Type
		{
			File,
			Url,
			Stream,
			Empty
		};

		AudioSource ();

		AudioSource (const QString&);
		AudioSource (const QUrl&);

		AudioSource (const AudioSource&);
		AudioSource& operator= (const AudioSource&);

		bool operator== (const AudioSource&) const;
		bool operator!= (const AudioSource&) const;

		QUrl ToUrl () const;

		bool IsLocalFile () const;
		QString GetLocalPath () const;

		bool IsRemote () const;

		bool IsEmpty () const;

		void Clear ();

		Type GetType () const;
	};

	uint qHash (const AudioSource&);

	typedef QList<AudioSource> AudioSources_t;
}
}

Q_DECLARE_METATYPE (LC::LMP::AudioSource)
Q_DECLARE_METATYPE (LC::LMP::AudioSources_t)
