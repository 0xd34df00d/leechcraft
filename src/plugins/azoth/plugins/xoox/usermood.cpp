/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "usermood.h"
#include <QDomElement>
#include <QXmppElement.h>

namespace LeechCraft
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

		for (int m = MoodEmpty + 1; m <= Worried; ++m)
			if (MoodStr [m] == str)
			{
				Mood_ = static_cast<Mood> (m);
				break;
			}
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
