/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QUrl>
#include "pepeventbase.h"

namespace Media
{
	struct AudioInfo;
}

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class UserTune : public PEPEventBase
	{
		QString Artist_;
		QString Source_;
		QString Title_;
		QString Track_;
		QUrl URI_;
		int Length_ = 0;
		int Rating_ = 0;
	public:
		static QString GetNodeString ();

		QXmppElement ToXML () const override;
		void Parse (const QDomElement&) override;
		QString Node () const override;

		PEPEventBase* Clone () const override;

		QString GetArtist () const;
		void SetArtist (const QString&);

		QString GetSource () const;
		void SetSource (const QString&);

		QString GetTitle () const;
		void SetTitle (const QString&);

		QString GetTrack () const;
		void SetTrack (const QString&);

		QUrl GetURI () const;
		void SetURI (const QUrl&);

		int GetLength () const;
		void SetLength (int);

		int GetRating () const;
		void SetRating (int);

		bool IsNull () const;

		Media::AudioInfo ToAudioInfo () const;
	};
}
}
}
