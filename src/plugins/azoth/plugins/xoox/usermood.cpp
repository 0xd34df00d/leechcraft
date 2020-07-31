/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "usermood.h"
#include <algorithm>
#include <QDomElement>
#include <QtDebug>
#include <QXmppElement.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsMoodNode = "http://jabber.org/protocol/mood";

	static const char* MoodStr[] =
	{
		"afraid",
		"amazed",
		"amorous",
		"angry",
		"annoyed",
		"anxious",
		"aroused",
		"ashamed",
		"bored",
		"brave",
		"calm",
		"cautious",
		"cold",
		"confident",
		"confused",
		"contemplative",
		"contented",
		"cranky",
		"crazy",
		"creative",
		"curious",
		"dejected",
		"depressed",
		"disappointed",
		"disgusted",
		"dismayed",
		"distracted",
		"embarrassed",
		"envious",
		"excited",
		"flirtatious",
		"frustrated",
		"grateful",
		"grieving",
		"grumpy",
		"guilty",
		"happy",
		"hopeful",
		"hot",
		"humbled",
		"humiliated",
		"hungry",
		"hurt",
		"impressed",
		"in_awe",
		"in_love",
		"indignant",
		"interested",
		"intoxicated",
		"invincible",
		"jealous",
		"lonely",
		"lost",
		"lucky",
		"mean",
		"moody",
		"nervous",
		"neutral",
		"offended",
		"outraged",
		"playful",
		"proud",
		"relaxed",
		"relieved",
		"remorseful",
		"restless",
		"sad",
		"sarcastic",
		"satisfied",
		"serious",
		"shocked",
		"shy",
		"sick",
		"sleepy",
		"spontaneous",
		"stressed",
		"strong",
		"surprised",
		"thankful",
		"thirsty",
		"tired",
		"undefined",
		"weak",
		"worried"
	};

	QString UserMood::GetNodeString ()
	{
		return NsMoodNode;
	}

	QXmppElement UserMood::ToXML () const
	{
		QXmppElement mood;
		mood.setTagName ("mood");
		mood.setAttribute ("xmlns", NsMoodNode);

		if (Mood_ != MoodEmpty)
		{
			QXmppElement elem;
			elem.setTagName (MoodStr [Mood_]);
			mood.appendChild (elem);

			if (!Text_.isEmpty ())
			{
				QXmppElement text;
				text.setTagName ("text");
				text.setValue (Text_);
				mood.appendChild (text);
			}
		}

		QXmppElement result;
		result.setTagName ("item");
		result.appendChild (mood);
		return result;
	}

	void UserMood::Parse (const QDomElement& elem)
	{
		Mood_ = MoodEmpty;
		Text_.clear ();

		QDomElement moodElem = elem.firstChildElement ("mood");
		if (moodElem.namespaceURI () != NsMoodNode)
			return;

		QDomElement mood = moodElem.firstChildElement ();
		while (!mood.isNull ())
		{
			const QString& tagName = mood.tagName ();
			if (tagName == "text")
				Text_ = mood.text ();
			else
				SetMoodStr (tagName);

			mood = mood.nextSiblingElement ();
		}
	}

	QString UserMood::Node () const
	{
		return GetNodeString ();
	}

	PEPEventBase* UserMood::Clone () const
	{
		return new UserMood (*this);
	}

	UserMood::Mood UserMood::GetMood () const
	{
		return Mood_;
	}

	void UserMood::SetMood (UserMood::Mood mood)
	{
		Mood_ = mood;
	}

	QString UserMood::GetMoodStr () const
	{
		return Mood_ == MoodEmpty ?
				QString () :
				MoodStr [Mood_];
	}

	void UserMood::SetMoodStr (const QString& str)
	{
		if (str.isEmpty ())
		{
			Mood_ = MoodEmpty;
			return;
		}

		const auto pos = std::find (std::begin (MoodStr), std::end (MoodStr), str);
		if (pos == std::end (MoodStr))
		{
			qWarning () << Q_FUNC_INFO
					<< str
					<< "not found";
			Mood_ = MoodEmpty;
		}
		else
			Mood_ = static_cast<Mood> (std::distance (std::begin (MoodStr), pos));
	}

	QString UserMood::GetText () const
	{
		return Text_;
	}

	void UserMood::SetText (const QString& text)
	{
		Text_ = text;
	}
}
}
}
