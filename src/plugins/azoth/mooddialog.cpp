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

#include "mooddialog.h"
#include <util/resourceloader.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
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
		
		Util::ResourceLoader *rl = Core::Instance ()
				.GetResourceLoader (Core::RLTMoodIconLoader);
		const QString& theme = XmlSettingsManager::Instance ()
				.property ("MoodIcons").toString () + '/';
				
		Ui_.Mood_->addItem (tr ("<clear>"));
		
		QMap<QString, QPair<QVariant, QIcon>> list;
		
		for (uint i = 0; i < sizeof (moodStr) / sizeof (moodStr [0]); ++i)
		{
			QString name (moodStr [i]);
			name [0] = name.at (0).toUpper ();
			QIcon icon (rl->GetIconPath (theme + name));
			
			list [tr (moodStr [i])] = qMakePair<QVariant, QIcon> (QString (moodStr [i]), icon);
		}
		
		Q_FOREACH (const QString& key, list.keys ())
			Ui_.Mood_->addItem (list [key].second,
					key, list [key].first);
	}
	
	QString MoodDialog::GetMood () const
	{
		return Ui_.Mood_->itemData (Ui_.Mood_->currentIndex ()).toString ();
	}
	
	void MoodDialog::SetMood (const QString& mood)
	{
		const int idx = std::max (0, Ui_.Mood_->findData (mood));
		Ui_.Mood_->setCurrentIndex (idx);
	}
	
	QString MoodDialog::GetText () const
	{
		return Ui_.Text_->text ();
	}
	
	void MoodDialog::SetText (const QString& text)
	{
		Ui_.Text_->setText (text);
	}
}
}
