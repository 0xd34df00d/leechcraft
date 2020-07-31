/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "useractivity.h"
#include <QDomElement>
#include <QXmppElement.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsActivityNode = "http://jabber.org/protocol/activity";

	static const char* activity_general[] =
	{
		"doing_chores",
		"drinking",
		"eating",
		"exercising",
		"grooming",
		"having_appointment",
		"inactive",
		"relaxing",
		"talking",
		"traveling",
		"working"
	};
	
	static const char* activity_specific[] =
	{
		"buying_groceries",
		"cleaning",
		"cooking",
		"doing_maintenance",
		"doing_the_dishes",
		"doing_the_laundry",
		"gardening",
		"running_an_errand",
		"walking_the_dog",
		"having_a_beer",
		"having_coffee",
		"having_tea",
		"having_a_snack",
		"having_breakfast",
		"having_dinner",
		"having_lunch",
		"cycling",
		"dancing",
		"hiking",
		"jogging",
		"playing_sports",
		"running",
		"skiing",
		"swimming",
		"working_out",
		"at_the_spa",
		"brushing_teeth",
		"getting_a_haircut",
		"shaving",
		"taking_a_bath",
		"taking_a_shower",
		"day_off",
		"hanging_out",
		"hiding",
		"on_vacation",
		"praying",
		"scheduled_holiday",
		"sleeping",
		"thinking",
		"fishing",
		"gaming",
		"going_out",
		"partying",
		"reading",
		"rehearsing",
		"shopping",
		"smoking",
		"socializing",
		"sunbathing",
		"watching_tv",
		"watching_a_movie",
		"in_real_life",
		"on_the_phone",
		"on_video_phone",
		"commuting",
		"cycling",
		"driving",
		"in_a_car",
		"on_a_bus",
		"on_a_plane",
		"on_a_train",
		"on_a_trip",
		"walking",
		"coding",
		"in_a_meeting",
		"studying",
		"writing",
		"other"
	};

	QString UserActivity::GetNodeString ()
	{
		return NsActivityNode;
	}
	
	QXmppElement UserActivity::ToXML () const
	{
		QXmppElement activityElem;
		activityElem.setTagName ("activity");
		activityElem.setAttribute ("xmlns", NsActivityNode);

		QXmppElement result;
		result.setTagName ("item");
		if (General_ == GeneralEmpty)
		{
			result.appendChild (activityElem);
			return result;
		}
		
		QXmppElement generalElem;
		generalElem.setTagName (activity_general [General_]);
		
		if (Specific_ != SpecificEmpty)
		{
			QXmppElement specific;
			specific.setTagName (activity_specific [Specific_]);
			generalElem.appendChild (specific);
		}
		
		if (!Text_.isEmpty ())
		{
			QXmppElement textElem;
			textElem.setTagName ("text");
			textElem.setValue (Text_);
			generalElem.appendChild (textElem);
		}

		activityElem.appendChild (generalElem);		
		result.appendChild (activityElem);

		return result;
	}
	
	void UserActivity::Parse (const QDomElement& item)
	{
		General_ = GeneralEmpty;
		Specific_ = SpecificEmpty;
		Text_.clear ();
		
		const QDomElement& activityElem = item.firstChildElement ("activity");
		QDomElement child = activityElem.firstChildElement ();
		while (!child.isNull ())
		{
			const QString& tagName = child.tagName ();
			if (tagName == "text")
			{
				Text_ = child.text ();
				child = child.nextSiblingElement ();
				continue;
			}
			
			for (int g = GeneralEmpty + 1; g <= Working; ++g)
			{
				if (tagName != activity_general [g])
					continue;

				General_ = static_cast<General> (g);

				const QDomElement& specific = child.firstChildElement ();
				if (!specific.isNull ())
					SetSpecificStr (specific.tagName ());

				break;
			}

			child = child.nextSiblingElement ();
		}
	}
	
	QString UserActivity::Node () const
	{
		return GetNodeString ();
	}

	PEPEventBase* UserActivity::Clone () const
	{
		return new UserActivity (*this);
	}
	
	UserActivity::General UserActivity::GetGeneral () const
	{
		return General_;
	}
	
	void UserActivity::SetGeneral (UserActivity::General general)
	{
		General_ = general;
	}
	
	QString UserActivity::GetGeneralStr () const
	{
		return General_ == GeneralEmpty ?
				QString () :
				activity_general [General_];
	}
	
	void UserActivity::SetGeneralStr (const QString& str)
	{
		General_ = GeneralEmpty;

		for (int g = GeneralEmpty + 1; g <= Working; ++g)
			if (str == activity_general [g])
			{
				General_ = static_cast<General> (g);
				break;
			}
	}
	
	UserActivity::Specific UserActivity::GetSpecific () const
	{
		return Specific_;
	}
	
	void UserActivity::SetSpecific (UserActivity::Specific specific)
	{
		Specific_ = specific;
	}
	
	void UserActivity::SetSpecificStr (const QString& str)
	{
		Specific_ = SpecificEmpty;
		
		for (int s = SpecificEmpty + 1; s <= Other; ++s)
			if (str == activity_specific [s])
			{
				Specific_ = static_cast<Specific> (s);
				break;
			}
	}
	
	QString UserActivity::GetSpecificStr () const
	{
		return Specific_ == SpecificEmpty ?
				QString () :
				activity_specific [Specific_];
	}
	
	QString UserActivity::GetText () const
	{
		return Text_;
	}
	
	void UserActivity::SetText (const QString& text)
	{
		Text_ = text;
	}
}
}
}
