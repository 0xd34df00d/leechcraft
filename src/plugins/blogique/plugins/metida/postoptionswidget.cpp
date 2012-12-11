/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "postoptionswidget.h"
#include <QtDebug>
#include <util/util.h>
#include "entryoptions.h"
#include "ljaccount.h"
#include "ljprofile.h"
#include "selectgroupsdialog.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	PostOptionsWidget::PostOptionsWidget (QWidget *parent)
	: QWidget (parent)
	, Account_ (0)
	, AllowMask_ (0)
	{
		Ui_.setupUi (this);

		FillItems ();
	}

	QString PostOptionsWidget::GetName () const
	{
		return tr ("Post options");
	}

	SideWidgetType PostOptionsWidget::GetWidgetType () const
	{
		return SideWidgetType::PostOptionsSideWidget;
	}

	QVariantMap PostOptionsWidget::GetPostOptions () const
	{
		QVariantMap map;

		map ["access"] = Ui_.Access_->itemData (Ui_.Access_->currentIndex (),
				Qt::UserRole);
		map ["allowMask"] = AllowMask_;

		const bool noMood = Ui_.Mood_->currentText ().isEmpty ();
		map ["noMood"] = noMood;

		if (!noMood)
		{
			int index = Ui_.Mood_->findText (Ui_.Mood_->currentText ());
			int moodId = Ui_.Mood_->itemData (index).toInt ();
			if (index == -1 ||
					!moodId)
				map ["mood"] = Ui_.Mood_->currentText ();
			else
				map ["moodId"] = moodId;
		}

		map ["place"] = Ui_.Place_->text ();
		map ["music"] = Ui_.Music_->text ();
		map ["comment"] = Ui_.Comments_->itemData (Ui_.Comments_->currentIndex (),
				Qt::UserRole);
		map ["notify"] = Ui_.NotifyAboutComments_->isChecked ();
		map ["hidecomment"] = Ui_.ScreenComments_->
				itemData (Ui_.ScreenComments_->currentIndex (), Qt::UserRole);
		map ["adults"] = Ui_.Adult_->itemData (Ui_.Adult_->currentIndex (),
				Qt::UserRole);
		map ["showInFriendsPage"] = Ui_.ShowInFriendsPage_->isChecked ();
		if (Ui_.UserPic_->currentIndex ())
			map ["avatar"] = Ui_.UserPic_->currentText ();

		return map;
	}

	void PostOptionsWidget::SetPostOptions (const QVariantMap& map)
	{
		const QVariant& access = map ["access"];
		for (int i = 0; i < Ui_.Access_->count (); ++i)
			if (Ui_.Access_->itemData (i, Qt::UserRole) == access)
			{
				Ui_.Access_->setCurrentIndex (i);
				break;
			}

		if (Ui_.Access_->itemData (Ui_.Access_->currentIndex ()) != Access::Private)
			Ui_.ShowInFriendsPage_->setChecked (map.contains ("showInFriendsPage") ?
				map ["showInFriendsPage"].toBool () : true);
		else
			Ui_.ShowInFriendsPage_->setChecked (false);

		//TODO AllowMask_

		if (map ["noMood"].toBool ())
			Ui_.Mood_->setCurrentIndex (-1);
		else
		{
			const QString& mood = map ["mood"].toString ();
			if (!mood.isEmpty ())
			{
				Ui_.Mood_->addItem (mood);
				Ui_.Mood_->setCurrentIndex (Ui_.Mood_->count () - 1);
			}
			else
			{
				int moodId = map ["moodId"].toInt ();
				for (int i = 0; i < Ui_.Mood_->count (); ++i)
					if (Ui_.Mood_->itemData (i, Qt::UserRole).toInt () == moodId)
					{
						Ui_.Mood_->setCurrentIndex (i);
						break;
					}
			}
		}

		Ui_.Place_->setText (map ["place"].toString ());
		Ui_.Music_->setText (map ["music"].toString ());

		const QVariant& comment = map ["comment"];
		for (int i = 0; i < Ui_.Comments_->count (); ++i)
			if (Ui_.Comments_->itemData (i, Qt::UserRole) == comment)
			{
				Ui_.Comments_->setCurrentIndex (i);
				break;
			}
		Ui_.NotifyAboutComments_->setChecked (map ["notify"].toBool ());

		const QVariant& hideComments = map ["hidecomment"];
		for (int i = 0; i < Ui_.ScreenComments_->count (); ++i)
			if (Ui_.ScreenComments_->itemData (i, Qt::UserRole) == hideComments)
			{
				Ui_.ScreenComments_->setCurrentIndex (i);
				break;
			}

		const QVariant& adults = map ["adults"];
		for (int i = 0; i < Ui_.Adult_->count (); ++i)
			if (Ui_.Adult_->itemData (i, Qt::UserRole) == adults)
			{
				Ui_.Adult_->setCurrentIndex (i);
				break;
			}

		Ui_.UserPic_->setCurrentIndex (!map ["avatar"].toString ().isEmpty () ?
			Ui_.UserPic_->findText (map ["avatar"].toString ()) :
			0);
		Ui_.NotifyAboutComments_->setChecked (map.contains ("notify") ?
			map ["notify"].toBool () :
			true);
	}

	QVariantMap PostOptionsWidget::GetCustomData () const
	{
		return QVariantMap ();
	}

	void PostOptionsWidget::SetCustomData (const QVariantMap&)
	{
	}

	void PostOptionsWidget::SetAccount (QObject *accObj)
	{
		Account_ = qobject_cast<LJAccount*> (accObj);
		if (!Account_)
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< accObj
					<< "doesn't belong to LiveJournal";
			return;
		}

		LJProfile *profile = qobject_cast<LJProfile*> (Account_->GetProfile ());
		if (!profile)
			return;

		Ui_.Mood_->clear ();
		for (const auto& mood : profile->GetProfileData ().Moods_)
			Ui_.Mood_->addItem (mood.Name_, mood.Id_);

		Ui_.UserPic_->addItem (tr ("(default)"));
		const QString& path = Util::CreateIfNotExists ("blogique/metida/avatars")
				.absoluteFilePath (Account_->GetAccountID ().toBase64 ().replace ('/', '_'));
		QPixmap pxm (path);
		Ui_.UserPicLabel_->setPixmap (pxm.scaled (64, 64));

		Ui_.UserPic_->addItems (profile->GetProfileData ().AvatarsID_);
	}

	QStringList PostOptionsWidget::GetTags () const
	{
		QStringList tags;
		for (auto tag : Ui_.Tags_->text ().split (","))
			tags << tag.trimmed ();
		return tags;
	}

	void PostOptionsWidget::SetTags (const QStringList& tags)
	{
		Ui_.Tags_->setText (tags.join (", "));
	}

	QDateTime PostOptionsWidget::GetPostDate () const
	{
		return !Ui_.TimestampBox_->isChecked () ?
				QDateTime::currentDateTime () :
				QDateTime (QDate (Ui_.Year_->value (),
							Ui_.Month_->currentIndex () + 1,
							Ui_.Date_->value ()),
						Ui_.Time_->time ());
	}

	void PostOptionsWidget::SetPostDate (const QDateTime& date)
	{
		Ui_.TimestampBox_->setChecked (true);
		Ui_.Year_->setValue (date.date ().year ());
		Ui_.Month_->setCurrentIndex (date.date ().month () - 1);
		Ui_.Date_->setValue (date.date ().day ());
		Ui_.Time_->setTime (date.time ());
	}

	void PostOptionsWidget::FillItems ()
	{
		Ui_.Access_->addItem (tr ("Public"), Access::Public);
		Ui_.Access_->addItem (tr ("Friends only"), Access::FriendsOnly);
		Ui_.Access_->addItem (tr ("Private"), Access::Private);
		Ui_.Access_->addItem (tr ("Custom"), Access::Custom);

		Ui_.Comments_->addItem (tr ("Enable"), CommentsManagement::EnableComments);
		Ui_.Comments_->addItem (tr ("Disable"), CommentsManagement::DisableComments);

		Ui_.ScreenComments_->addItem (tr ("Default"), CommentsManagement::Default);
		Ui_.ScreenComments_->addItem (tr ("Anonymouse only"),
				CommentsManagement::ScreenAnonymouseComments);
		Ui_.ScreenComments_->addItem (tr ("Not from friends"),
				CommentsManagement::ShowFriendsComments);
		Ui_.ScreenComments_->addItem (tr ("Not from friends with links"),
				CommentsManagement::ScreenNotFromFriendsWithLinks);
		Ui_.ScreenComments_->addItem (tr ("Don't hide'"),
				CommentsManagement::ShowComments);
		Ui_.ScreenComments_->addItem (tr ("All"),
				CommentsManagement::ScreenComments);

		Ui_.Adult_->addItem (tr ("Without adult content"),
				AdultContent::WithoutAdultContent);
		Ui_.Adult_->addItem (tr ("For adults (>14)"), AdultContent::AdultsFrom14);
		Ui_.Adult_->addItem (tr ("For adults (>18)"), AdultContent::AdultsFrom18);
	}


	void PostOptionsWidget::on_CurrentTime__released ()
	{
		QDateTime current = QDateTime::currentDateTime ();
		Ui_.Year_->setValue (current.date ().year ());
		Ui_.Month_->setCurrentIndex (current.date ().month () - 1);
		Ui_.Date_->setValue (current.date ().day ());
		Ui_.Time_->setTime (current.time ());
	}

	void PostOptionsWidget::on_Access__activated (int index)
	{
		if (static_cast<Access> (Ui_.Access_->itemData (index).toInt ()) == Access::Custom)
		{
			SelectGroupsDialog dlg (qobject_cast<LJProfile*> (Account_->GetProfile ()),
					AllowMask_);

			if (dlg.exec () == QDialog::Rejected ||
					dlg.GetSelectedGroupsIds ().isEmpty ())
				Ui_.Access_->setCurrentIndex (0);
			else
				for (uint num : dlg.GetSelectedGroupsIds ())
					AllowMask_ |= 1 << num;
		}
	}

	void PostOptionsWidget::on_UserPic__currentIndexChanged (int index)
	{
		QString path = index ?
			Util::CreateIfNotExists ("blogique/metida/avatars")
				.absoluteFilePath ((Account_->GetAccountID () +
					Ui_.UserPic_->itemText (index).toUtf8 ())
						.toBase64 ().replace ('/', '_')) :
			Util::CreateIfNotExists ("blogique/metida/avatars")
						.absoluteFilePath ((Account_->GetAccountID ())
								.toBase64 ().replace ('/', '_'));
		QPixmap pxm (path);
		Ui_.UserPicLabel_->setPixmap (pxm.scaled (pxm.width (), pxm.height ()));
	}
}
}
}
