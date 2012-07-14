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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERMOOD_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERMOOD_H
#include <QString>
#include "pepeventbase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class UserMood : public PEPEventBase
	{
	public:
		enum Mood
		{
			MoodEmpty = -1,
			Afraid,
			Amazed,
			Amorous,
			Angry,
			Anoyed,
			Anxious,
			Aroused,
			Ashamed,
			Bored,
			Brave,
			Calm,
			Cautious,
			Cold,
			Confident,
			Confused,
			Contemplative,
			Contented,
			Cranky,
			Crazy,
			Creative,
			Curious,
			Dejected,
			Depressed,
			Disappointed,
			Disgusted,
			Dismayed,
			Distracted,
			Embarrassed,
			Envious,
			Excited,
			Flirtatious,
			Frustrated,
			Grateful,
			Grieving,
			Grumpy,
			Guilty,
			Happy,
			Hopeful,
			Hot,
			Humbled,
			Humiliated,
			Hungry,
			Hurt,
			Impressed,
			In_awe,
			In_love,
			Indignant,
			Interested,
			Intoxicated,
			Invincible,
			Jealous,
			Lonely,
			Lost,
			Lucky,
			Mean,
			Moody,
			Nervous,
			Neutral,
			Offended,
			Outraged,
			Playful,
			Proud,
			Relaxed,
			Relieved,
			Remorseful,
			Restless,
			Sad,
			Sarcastic,
			Satisfied,
			Serious,
			Shocked,
			Shy,
			Sick,
			Sleepy,
			Spontaneous,
			Stressed,
			Strong,
			Surprised,
			Thankful,
			Thirsty,
			Tired,
			Undefined,
			Weak,
			Worried
		};
	private:
		Mood Mood_;
		QString Text_;
	public:
		static QString GetNodeString ();
		
		QXmppElement ToXML () const;
		void Parse (const QDomElement&);
		QString Node () const;
		
		PEPEventBase* Clone () const;
		
		Mood GetMood () const;
		void SetMood (Mood);
		QString GetMoodStr () const;
		void SetMoodStr (const QString&);
		
		QString GetText () const;
		void SetText (const QString&);
	};
}
}
}

#endif
