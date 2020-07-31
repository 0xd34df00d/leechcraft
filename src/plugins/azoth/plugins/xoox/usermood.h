/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERMOOD_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERMOOD_H
#include <QString>
#include "pepeventbase.h"

namespace LC
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
		
		QXmppElement ToXML () const override;
		void Parse (const QDomElement&) override;
		QString Node () const override;
		
		PEPEventBase* Clone () const override;
		
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
