/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERACTIVITY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERACTIVITY_H
#include <QString>
#include "pepeventbase.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class UserActivity : public PEPEventBase
	{
	public:
		enum General
		{
			GeneralEmpty = -1,
			DoingChores,
			Drinking,
			Eating,
			Exercising,
			Grooming,
			HavingAppointment,
			Inactive,
			Relaxing,
			Talking,
			Traveling,
			Working
		};
		
		enum Specific
		{
			SpecificEmpty = -1,
			BuyingGroceries,
			Cleaning,
			Cooking,
			DoingMaintenance,
			DoingTheDishes,
			DoingTheLaundry,
			Gardening,
			RunningAnErrand,
			WalkingTheDog,
			HavingABeer,
			HavingCoffee,
			HavingTea,
			HavingASnack,
			HavingBreakfast,
			HavingDinner,
			HavingLunch,
			Dancing,
			Hiking,
			Jogging,
			PlayingSports,
			Running,
			Skiing,
			Swimming,
			WorkingOut,
			AtTheSpa,
			BrushingTeeth,
			GettingAHaircut,
			Shaving,
			TakingABath,
			TakingAShower,
			DayOff,
			HangingOut,
			Hiding,
			OnVacation,
			Praying,
			ScheduledHoliday,
			Sleeping,
			Thinking,
			Fishing,
			Gaming,
			GoingOut,
			Partying,
			Reading,
			Rehearsing,
			Shopping,
			Smoking,
			Socializing,
			Sunbathing,
			WatchingTv,
			WatchingAMovie,
			InRealLife,
			OnThePhone,
			OnVideoPhone,
			Commuting,
			Cycling,
			Driving,
			InACar,
			OnABus,
			OnAPlane,
			OnATrain,
			OnATrip,
			Walking,
			Coding,
			InAMeeting,
			Studying,
			Writing,
			Other // any other activity (without further specification) not defined herein
		};
	private:
		General General_;
		Specific Specific_;
		QString Text_;
	public:
		static QString GetNodeString ();
		
		QXmppElement ToXML () const override;
		void Parse (const QDomElement&) override;
		QString Node () const override;
		
		PEPEventBase* Clone () const override;
		
		General GetGeneral () const;
		void SetGeneral (General);
		QString GetGeneralStr () const;
		void SetGeneralStr (const QString&);
		
		Specific GetSpecific () const;
		void SetSpecific (Specific);
		QString GetSpecificStr () const;
		void SetSpecificStr (const QString&);
		
		QString GetText () const;
		void SetText (const QString&);
	};
}
}
}

#endif
