/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "usertune.h"
#include <QDomElement>
#include <QXmppElement.h>
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsTuneNode = "http://jabber.org/protocol/tune";

	QString UserTune::GetNodeString ()
	{
		return NsTuneNode;
	}

	QXmppElement UserTune::ToXML () const
	{
		QXmppElement result;
		result.setTagName ("item");

		QXmppElement tune;
		tune.setTagName ("tune");
		tune.setAttribute ("xmlns", NsTuneNode);

		auto appendTxt = [&tune] (const QString& tag, const QString& str)
		{
			if (str.isEmpty ())
				return;

			QXmppElement elem;
			elem.setTagName (tag);
			elem.setValue (str);
			tune.appendChild (elem);
		};

		appendTxt ("artist", Artist_);
		appendTxt ("source", Source_);
		appendTxt ("title", Title_);
		appendTxt ("track", Track_);
		appendTxt ("uri", URI_.toEncoded ());
		if (Length_)
			appendTxt ("length", QString::number (Length_));
		if (Rating_)
			appendTxt ("rating", QString::number (Rating_));

		result.appendChild (tune);

		return result;
	}

	void UserTune::Parse (const QDomElement& elem)
	{
		QDomElement tune = elem.firstChildElement ("tune");
		if (tune.namespaceURI () != NsTuneNode)
			return;

		Artist_ = tune.firstChildElement ("artist").text ();
		Source_ = tune.firstChildElement ("source").text ();
		Title_ = tune.firstChildElement ("title").text ();
		Track_ = tune.firstChildElement ("track").text ();
		URI_ = QUrl::fromEncoded (tune.firstChildElement ("uri").text ().toUtf8 ());
		Length_ = tune.firstChildElement ("length").text ().toInt ();
		Rating_ = tune.firstChildElement ("rating").text ().toInt ();
	}

	QString UserTune::Node () const
	{
		return GetNodeString ();
	}

	PEPEventBase* UserTune::Clone () const
	{
		return new UserTune (*this);
	}

	QString UserTune::GetArtist () const
	{
		return Artist_;
	}

	void UserTune::SetArtist (const QString& artist)
	{
		Artist_ = artist;
	}

	QString UserTune::GetSource () const
	{
		return Source_;
	}

	void UserTune::SetSource (const QString& source)
	{
		Source_ = source;
	}

	QString UserTune::GetTitle () const
	{
		return Title_;
	}

	void UserTune::SetTitle (const QString& title)
	{
		Title_ = title;
	}

	QString UserTune::GetTrack () const
	{
		return Track_;
	}

	void UserTune::SetTrack (const QString& track)
	{
		Track_ = track;
	}

	QUrl UserTune::GetURI () const
	{
		return URI_;
	}

	void UserTune::SetURI (const QUrl& uri)
	{
		URI_ = uri;
	}

	int UserTune::GetLength () const
	{
		return Length_;
	}

	void UserTune::SetLength (int length)
	{
		Length_ = length;
	}

	int UserTune::GetRating () const
	{
		return Rating_;
	}

	void UserTune::SetRating (int rating)
	{
		Rating_ = rating;
	}

	bool UserTune::IsNull () const
	{
		return Artist_.isEmpty () &&
				Title_.isEmpty () &&
				Track_.isEmpty () &&
				Source_.isEmpty ();
	}

	Media::AudioInfo UserTune::ToAudioInfo () const
	{
		return
		{
			GetArtist (),
			GetSource (),
			GetTitle (),
			{},
			GetLength (),
			0,
			GetTrack ().toInt (),
			{ { "URL", GetURI () } }
		};
	}
}
}
}
