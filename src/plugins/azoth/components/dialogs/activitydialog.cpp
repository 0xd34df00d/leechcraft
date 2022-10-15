/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "activitydialog.h"
#include <util/sys/resourceloader.h>
#include <util/sll/qtutil.h>
#include <util/sll/void.h>
#include "interfaces/azoth/activityinfo.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "resourcesmanager.h"

namespace LC
{
namespace Azoth
{
	template<typename T>
	struct WithInfo
	{
		QIcon Icon_;
		QString RawName_;
		T Payload_;
	};

	auto ActivityDialog::GetActivityInfos ()
	{
		static const auto infos = []
		{
			const char *genAct [] =
			{
				QT_TR_NOOP ("doing_chores"),
				QT_TR_NOOP ("drinking"),
				QT_TR_NOOP ("eating"),
				QT_TR_NOOP ("exercising"),
				QT_TR_NOOP ("grooming"),
				QT_TR_NOOP ("having_appointment"),
				QT_TR_NOOP ("inactive"),
				QT_TR_NOOP ("relaxing"),
				QT_TR_NOOP ("talking"),
				QT_TR_NOOP ("traveling"),
				QT_TR_NOOP ("working")
			};

			const char *specAct [] =
			{
				QT_TR_NOOP ("buying_groceries"),
				QT_TR_NOOP ("cleaning"),
				QT_TR_NOOP ("cooking"),
				QT_TR_NOOP ("doing_maintenance"),
				QT_TR_NOOP ("doing_the_dishes"),
				QT_TR_NOOP ("doing_the_laundry"),
				QT_TR_NOOP ("gardening"),
				QT_TR_NOOP ("running_an_errand"),
				QT_TR_NOOP ("walking_the_dog"),
				QT_TR_NOOP ("having_a_beer"),
				QT_TR_NOOP ("having_coffee"),
				QT_TR_NOOP ("having_tea"),
				QT_TR_NOOP ("having_a_snack"),
				QT_TR_NOOP ("having_breakfast"),
				QT_TR_NOOP ("having_dinner"),
				QT_TR_NOOP ("having_lunch"),
				QT_TR_NOOP ("cycling"),
				QT_TR_NOOP ("dancing"),
				QT_TR_NOOP ("hiking"),
				QT_TR_NOOP ("jogging"),
				QT_TR_NOOP ("playing_sports"),
				QT_TR_NOOP ("running"),
				QT_TR_NOOP ("skiing"),
				QT_TR_NOOP ("swimming"),
				QT_TR_NOOP ("working_out"),
				QT_TR_NOOP ("at_the_spa"),
				QT_TR_NOOP ("brushing_teeth"),
				QT_TR_NOOP ("getting_a_haircut"),
				QT_TR_NOOP ("shaving"),
				QT_TR_NOOP ("taking_a_bath"),
				QT_TR_NOOP ("taking_a_shower"),
				QT_TR_NOOP ("day_off"),
				QT_TR_NOOP ("hanging_out"),
				QT_TR_NOOP ("hiding"),
				QT_TR_NOOP ("on_vacation"),
				QT_TR_NOOP ("praying"),
				QT_TR_NOOP ("scheduled_holiday"),
				QT_TR_NOOP ("sleeping"),
				QT_TR_NOOP ("thinking"),
				QT_TR_NOOP ("fishing"),
				QT_TR_NOOP ("gaming"),
				QT_TR_NOOP ("going_out"),
				QT_TR_NOOP ("partying"),
				QT_TR_NOOP ("reading"),
				QT_TR_NOOP ("rehearsing"),
				QT_TR_NOOP ("shopping"),
				QT_TR_NOOP ("smoking"),
				QT_TR_NOOP ("socializing"),
				QT_TR_NOOP ("sunbathing"),
				QT_TR_NOOP ("watching_tv"),
				QT_TR_NOOP ("watching_a_movie"),
				QT_TR_NOOP ("in_real_life"),
				QT_TR_NOOP ("on_the_phone"),
				QT_TR_NOOP ("on_video_phone"),
				QT_TR_NOOP ("commuting"),
				QT_TR_NOOP ("cycling"),
				QT_TR_NOOP ("driving"),
				QT_TR_NOOP ("in_a_car"),
				QT_TR_NOOP ("on_a_bus"),
				QT_TR_NOOP ("on_a_plane"),
				QT_TR_NOOP ("on_a_train"),
				QT_TR_NOOP ("on_a_trip"),
				QT_TR_NOOP ("walking"),
				QT_TR_NOOP ("coding"),
				QT_TR_NOOP ("in_a_meeting"),
				QT_TR_NOOP ("studying"),
				QT_TR_NOOP ("writing"),
				QT_TR_NOOP ("other")
			};

			auto rl = ResourcesManager::Instance ().GetResourceLoader (ResourcesManager::RLTActivityIconLoader);
			const auto& theme = XmlSettingsManager::Instance ().property ("ActivityIcons").toString () + '/';

			using SpecificList_t = QMap<QString, WithInfo<Util::Void>>;
			QMap<QString, WithInfo<SpecificList_t>> infos;

			int sizes [] = { 9, 3, 4, 9, 6, 0, 8, 12, 3, 9, 4 };
			for (size_t i = 0, pos = 0; pos < sizeof (sizes) / sizeof (sizes [0]); ++pos)
			{
				SpecificList_t specifics;

				const auto& iconPrefix = theme + genAct [pos] + '_';
				for (int j = 0; j < sizes [pos]; ++j, ++i)
				{
					QIcon icon { rl->GetIconPath (iconPrefix + specAct [i]) };
					specifics [tr (specAct [i])] = { icon, specAct [i], {} };
				}

				infos [tr (genAct [pos])] =
				{
					QIcon { rl->GetIconPath (theme + genAct [pos]) },
					genAct [pos],
					std::move (specifics)
				};
			}

			return infos;
		} ();

		return infos;
	}

	QString ActivityDialog::ToHumanReadable (const QString& str)
	{
		return tr (str.toLatin1 ());
	}

	ActivityDialog::ActivityDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Ui_.ActivityTree_->addTopLevelItem (new QTreeWidgetItem { { tr ("<clear>") } });

		for (const auto& [name, specifics] : Util::Stlize (GetActivityInfos ()))
		{
			const auto item = new QTreeWidgetItem { { name } };
			item->setIcon (0, specifics.Icon_);
			item->setData (0, Qt::UserRole, QVariant::fromValue (ActivityInfo { specifics.RawName_, {}, {} }));

			for (const auto& [specificName, specificInfo] : Util::Stlize (specifics.Payload_))
			{
				const auto subItem = new QTreeWidgetItem { { specificName } };
				subItem->setIcon (0, specificInfo.Icon_);
				subItem->setData (0, Qt::UserRole,
						QVariant::fromValue (ActivityInfo { specifics.RawName_, specificInfo.RawName_, {} }));
				item->addChild (subItem);
			}

			Ui_.ActivityTree_->addTopLevelItem (item);
		}

		Ui_.ActivityTree_->expandAll ();
	}

	ActivityInfo ActivityDialog::GetActivityInfo () const
	{
		auto result = Ui_.ActivityTree_->currentIndex ().data (Qt::UserRole).value<ActivityInfo> ();
		result.Text_ = Ui_.Text_->text ();
		return result;
	}

	void ActivityDialog::SetActivityInfo (const ActivityInfo& info)
	{
		SetCurrentActivityItem (info);
		Ui_.Text_->setText (info.Text_);
	}

	void ActivityDialog::SetCurrentActivityItem (const ActivityInfo& info)
	{
		Ui_.ActivityTree_->setCurrentItem (Ui_.ActivityTree_->topLevelItem (0));
		for (int row = 0; row < Ui_.ActivityTree_->topLevelItemCount (); ++row)
		{
			const auto& topLevelItem = Ui_.ActivityTree_->topLevelItem (row);
			if (topLevelItem->data (0, Qt::UserRole).value<ActivityInfo> ().General_ != info.General_)
				continue;

			for (int subRow = 0; subRow < topLevelItem->childCount (); ++subRow)
			{
				const auto childItem = topLevelItem->child (subRow);
				if (childItem->data (0, Qt::UserRole).value<ActivityInfo> ().Specific_ == info.Specific_)
				{
					Ui_.ActivityTree_->setCurrentItem (childItem);
					return;
				}
			}
			return;
		}
	}
}
}
