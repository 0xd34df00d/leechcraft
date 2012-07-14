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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERACTIVITY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERACTIVITY_H
#include <QString>
#include "pepeventbase.h"

namespace LeechCraft
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
		
		QXmppElement ToXML () const;
		void Parse (const QDomElement&);
		QString Node () const;
		
		PEPEventBase* Clone () const;
		
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
