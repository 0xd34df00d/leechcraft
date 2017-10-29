/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "activitydialog.h"
#include <util/sys/resourceloader.h>
#include <util/sll/qtutil.h>
#include <util/sll/void.h>
#include "interfaces/azoth/activityinfo.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "resourcesmanager.h"

namespace LeechCraft
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

		Ui_.General_->addItem (tr ("<clear>"));

		for (const auto& [name, specifics] : Util::Stlize (GetActivityInfos ()))
			Ui_.General_->addItem (specifics.Icon_, name, specifics.RawName_);
	}

	QString ActivityDialog::GetGeneral () const
	{
		return Ui_.General_->itemData (Ui_.General_->currentIndex ()).toString ();
	}

	void ActivityDialog::SetGeneral (const QString& general)
	{
		const int idx = std::max (0, Ui_.General_->findData (general));
		Ui_.General_->setCurrentIndex (idx);
		on_General__currentIndexChanged (idx);
	}

	QString ActivityDialog::GetSpecific () const
	{
		return Ui_.Specific_->itemData (Ui_.Specific_->currentIndex ()).toString ();
	}

	void ActivityDialog::SetSpecific (const QString& specific)
	{
		const int idx = std::max (0, Ui_.Specific_->findData (specific));
		Ui_.Specific_->setCurrentIndex (idx);
	}

	QString ActivityDialog::GetText () const
	{
		return Ui_.Text_->text ();
	}

	void ActivityDialog::SetText (const QString& text)
	{
		Ui_.Text_->setText (text);
	}

	void ActivityDialog::on_General__currentIndexChanged (int idx)
	{
		Ui_.Specific_->clear ();

		if (!idx)
			return;

		const auto& general = Ui_.General_->currentText ();
		const auto& specifics = GetActivityInfos () [general].Payload_;

		for (const auto& [name, info] : Util::Stlize (specifics))
			Ui_.Specific_->addItem (info.Icon_, name, info.RawName_);
	}
}
}
