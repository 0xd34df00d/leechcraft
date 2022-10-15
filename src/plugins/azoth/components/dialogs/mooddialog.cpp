/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mooddialog.h"
#include <util/sys/resourceloader.h>
#include <util/sll/qtutil.h>
#include "interfaces/azoth/moodinfo.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "resourcesmanager.h"

namespace LC
{
namespace Azoth
{
	QString MoodDialog::ToHumanReadable (const QString& str)
	{
		return tr (str.toLatin1 ());
	}

	MoodDialog::MoodDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Ui_.Mood_->addTopLevelItem (new QTreeWidgetItem { { tr ("<clear>") } });

		for (const auto& pair : Util::Stlize (BuildHumanReadableList ()))
		{
			const auto item = new QTreeWidgetItem { { pair.first } };
			item->setIcon (0, pair.second.second);
			item->setData (0, Qt::UserRole, pair.second.first);
			Ui_.Mood_->addTopLevelItem (item);
		}
	}

	MoodInfo MoodDialog::GetMood () const
	{
		return { Ui_.Mood_->currentIndex ().data (Qt::UserRole).toString (), Ui_.Text_->text () };
	}

	void MoodDialog::SetMood (const MoodInfo& moodInfo)
	{
		const auto& mood = moodInfo.Mood_;
		const auto& list = BuildHumanReadableList ();
		const auto pos = std::find_if (list.begin (), list.end (),
				[&mood] (const auto& pair) { return mood == pair.first; });
		const auto idx = pos == list.end () ?
				0 :
				std::distance (list.begin (), pos) + 1;
		Ui_.Mood_->setCurrentItem (Ui_.Mood_->topLevelItem (idx));

		Ui_.Text_->setText (moodInfo.Text_);
	}

	QMap<QString, QPair<QVariant, QIcon>> MoodDialog::BuildHumanReadableList ()
	{
		static const auto list = []
		{
			const char* moodStr[] =
			{
				QT_TR_NOOP ("afraid"),
				QT_TR_NOOP ("amazed"),
				QT_TR_NOOP ("amorous"),
				QT_TR_NOOP ("angry"),
				QT_TR_NOOP ("annoyed"),
				QT_TR_NOOP ("anxious"),
				QT_TR_NOOP ("aroused"),
				QT_TR_NOOP ("ashamed"),
				QT_TR_NOOP ("bored"),
				QT_TR_NOOP ("brave"),
				QT_TR_NOOP ("calm"),
				QT_TR_NOOP ("cautious"),
				QT_TR_NOOP ("cold"),
				QT_TR_NOOP ("confident"),
				QT_TR_NOOP ("confused"),
				QT_TR_NOOP ("contemplative"),
				QT_TR_NOOP ("contented"),
				QT_TR_NOOP ("cranky"),
				QT_TR_NOOP ("crazy"),
				QT_TR_NOOP ("creative"),
				QT_TR_NOOP ("curious"),
				QT_TR_NOOP ("dejected"),
				QT_TR_NOOP ("depressed"),
				QT_TR_NOOP ("disappointed"),
				QT_TR_NOOP ("disgusted"),
				QT_TR_NOOP ("dismayed"),
				QT_TR_NOOP ("distracted"),
				QT_TR_NOOP ("embarrassed"),
				QT_TR_NOOP ("envious"),
				QT_TR_NOOP ("excited"),
				QT_TR_NOOP ("flirtatious"),
				QT_TR_NOOP ("frustrated"),
				QT_TR_NOOP ("grateful"),
				QT_TR_NOOP ("grieving"),
				QT_TR_NOOP ("grumpy"),
				QT_TR_NOOP ("guilty"),
				QT_TR_NOOP ("happy"),
				QT_TR_NOOP ("hopeful"),
				QT_TR_NOOP ("hot"),
				QT_TR_NOOP ("humbled"),
				QT_TR_NOOP ("humiliated"),
				QT_TR_NOOP ("hungry"),
				QT_TR_NOOP ("hurt"),
				QT_TR_NOOP ("impressed"),
				QT_TR_NOOP ("in_awe"),
				QT_TR_NOOP ("in_love"),
				QT_TR_NOOP ("indignant"),
				QT_TR_NOOP ("interested"),
				QT_TR_NOOP ("intoxicated"),
				QT_TR_NOOP ("invincible"),
				QT_TR_NOOP ("jealous"),
				QT_TR_NOOP ("lonely"),
				QT_TR_NOOP ("lost"),
				QT_TR_NOOP ("lucky"),
				QT_TR_NOOP ("mean"),
				QT_TR_NOOP ("moody"),
				QT_TR_NOOP ("nervous"),
				QT_TR_NOOP ("neutral"),
				QT_TR_NOOP ("offended"),
				QT_TR_NOOP ("outraged"),
				QT_TR_NOOP ("playful"),
				QT_TR_NOOP ("proud"),
				QT_TR_NOOP ("relaxed"),
				QT_TR_NOOP ("relieved"),
				QT_TR_NOOP ("remorseful"),
				QT_TR_NOOP ("restless"),
				QT_TR_NOOP ("sad"),
				QT_TR_NOOP ("sarcastic"),
				QT_TR_NOOP ("satisfied"),
				QT_TR_NOOP ("serious"),
				QT_TR_NOOP ("shocked"),
				QT_TR_NOOP ("shy"),
				QT_TR_NOOP ("sick"),
				QT_TR_NOOP ("sleepy"),
				QT_TR_NOOP ("spontaneous"),
				QT_TR_NOOP ("stressed"),
				QT_TR_NOOP ("strong"),
				QT_TR_NOOP ("surprised"),
				QT_TR_NOOP ("thankful"),
				QT_TR_NOOP ("thirsty"),
				QT_TR_NOOP ("tired"),
				QT_TR_NOOP ("undefined"),
				QT_TR_NOOP ("weak"),
				QT_TR_NOOP ("worried")
			};

			const auto& theme = XmlSettingsManager::Instance ().property ("MoodIcons").toString () + '/';
			const auto rl = ResourcesManager::Instance ().GetResourceLoader (ResourcesManager::RLTMoodIconLoader);

			QMap<QString, QPair<QVariant, QIcon>> list;
			for (uint i = 0; i < sizeof (moodStr) / sizeof (moodStr [0]); ++i)
			{
				QString name (moodStr [i]);
				name [0] = name.at (0).toUpper ();
				QIcon icon (rl->GetIconPath (theme + name));

				list [tr (moodStr [i])] = qMakePair<QVariant, QIcon> (QString (moodStr [i]), icon);
			}
			return list;
		} ();
		return list;
	}
}
}
