/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "postoptionswidget.h"
#include <QDir>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <util/qml/themeimageprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/icurrentsongkeeper.h>
#include "entryoptions.h"
#include "ljaccount.h"
#include "ljprofile.h"
#include "selectgroupsdialog.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	PostOptionsWidget::PostOptionsWidget (const ICoreProxy_ptr& proxy, QWidget *parent)
	: QWidget { parent }
	, Proxy_ { proxy }
	{
		Ui_.setupUi (this);

		XmlSettingsManager::Instance ().RegisterObject ("AutoUpdateCurrentMusic",
				this, "handleAutoUpdateCurrentMusic");
		handleAutoUpdateCurrentMusic ();
		FillItems ();

		connect (Ui_.HideMainOptions_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleHideMainOptions (bool)));
		connect (Ui_.HideLikeButtons_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleHideLikeButtons (bool)));
		Ui_.HideMainOptions_->setChecked (!XmlSettingsManager::Instance ()
				.Property ("CollapseMainOptions", false).toBool ());
		Ui_.HideLikeButtons_->setChecked (!XmlSettingsManager::Instance ()
				.Property ("CollapseLikeButtons", false).toBool ());
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

		const bool customMood = !Ui_.Mood_->itemData (Ui_.Mood_->currentIndex ()).isValid ();
		if (!customMood)
			map ["moodId"] = Ui_.Mood_->itemData (Ui_.Mood_->currentIndex ()).toInt ();
		else
		{
			map ["mood"] = Ui_.Mood_->currentText ();
			map ["moodId"] = -1;
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

		QStringList likes;

		if (Ui_.VkontakteLike_->isChecked ())
			likes << "vkontakte";
		if (Ui_.FacebookLike_->isChecked ())
			likes << "facebook";
		if (Ui_.GoogleLike_->isChecked ())
			likes << "google";
		if (Ui_.TwitterLike_->isChecked ())
			likes << "twitter";
		if (Ui_.LiveJournalReward_->isChecked ())
			likes << "livejournal";
		if (Ui_.LiveJournalRepost_->isChecked ())
			likes << "repost";
		if (Ui_.TumblrLike_->isChecked ())
			likes << "tumblr";
		if (Ui_.SurfingbirdLike_->isChecked ())
			likes << "surfingbird";

		map ["likes"] = likes;


		if (XmlSettingsManager::Instance ().Property ("SaveSelectedButtons", true).toBool ())
			XmlSettingsManager::Instance ().setProperty ("SavedLikeButtons", likes);

		return map;
	}

	void PostOptionsWidget::SetPostOptions (const QVariantMap& map)
	{
		if (map.contains ("access"))
		{
			const QVariant& access = map ["access"];
			for (int i = 0; i < Ui_.Access_->count (); ++i)
				if (Ui_.Access_->itemData (i, Qt::UserRole) == access)
				{
					Ui_.Access_->setCurrentIndex (i);
					break;
				}
		}
		else
			Ui_.Access_->setCurrentIndex (0);

		if (Ui_.Access_->itemData (Ui_.Access_->currentIndex ()) != Access::Private)
			Ui_.ShowInFriendsPage_->setChecked (map.contains ("showInFriendsPage") ?
				map ["showInFriendsPage"].toBool () : true);
		else
			Ui_.ShowInFriendsPage_->setChecked (false);

		//TODO AllowMask_

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
				if (Ui_.Mood_->itemData (i).toInt () == moodId)
				{
					Ui_.Mood_->setCurrentIndex (i);
					break;
				}
		}

		Ui_.Place_->setText (map ["place"].toString ());
		Ui_.Music_->setText (map ["music"].toString ());

		if (map.contains ("comment"))
		{
			const QVariant& comment = map ["comment"];
			for (int i = 0; i < Ui_.Comments_->count (); ++i)
				if (Ui_.Comments_->itemData (i, Qt::UserRole) == comment)
				{
					Ui_.Comments_->setCurrentIndex (i);
					break;
				}
		}
		else
			Ui_.Comments_->setCurrentIndex (0);

		if (map.contains ("hidecomments"))
		{
			const QVariant& hideComments = map ["hidecomment"];
			for (int i = 0; i < Ui_.ScreenComments_->count (); ++i)
				if (Ui_.ScreenComments_->itemData (i, Qt::UserRole) == hideComments)
				{
					Ui_.ScreenComments_->setCurrentIndex (i);
					break;
				}
		}
		else
			Ui_.ScreenComments_->setCurrentIndex (0);

		if (map.contains ("adults"))
		{
			const QVariant& adults = map ["adults"];
			for (int i = 0; i < Ui_.Adult_->count (); ++i)
				if (Ui_.Adult_->itemData (i, Qt::UserRole) == adults)
				{
					Ui_.Adult_->setCurrentIndex (i);
					break;
				}
		}
		else
			Ui_.Adult_->setCurrentIndex (0);

		Ui_.UserPic_->setCurrentIndex (!map ["avatar"].toString ().isEmpty () ?
			Ui_.UserPic_->findText (map ["avatar"].toString ()) :
			0);
		Ui_.NotifyAboutComments_->setChecked (map.contains ("notify") ?
			map ["notify"].toBool () :
			true);

		if (map.contains ("content"))
		{
			static const QRegularExpression rxp
			{
				"<lj-like\\s?(buttons=\"((\\w+,?)+)\"\\s?)?\\/?>",
				QRegularExpression::CaseInsensitiveOption
			};
			const auto& match = rxp.match (map ["content"].toString ());
			if (match.capturedTexts ().count () == 1)
			{
				for (const auto button : { Ui_.VkontakteLike_, Ui_.FacebookLike_, Ui_.GoogleLike_,
											Ui_.LiveJournalReward_, Ui_.LiveJournalRepost_, Ui_.TwitterLike_,
											Ui_.TumblrLike_, Ui_.SurfingbirdLike_ })
					button->setChecked (true);
				Ui_.AllLike_->setChecked (true);
			}
			else if (match.hasMatch ())
			{
				const auto likes = match.capturedView (2).split (',');
				Ui_.VkontakteLike_->setChecked (likes.contains (u"vkontakte"_qsv));
				Ui_.FacebookLike_->setChecked (likes.contains (u"facebook"_qsv));
				Ui_.GoogleLike_->setChecked (likes.contains (u"google"_qsv));
				Ui_.LiveJournalReward_->setChecked (likes.contains (u"livejournal"_qsv));
				Ui_.LiveJournalRepost_->setChecked (likes.contains (u"repost"_qsv));
				Ui_.TwitterLike_->setChecked (likes.contains (u"twitter"_qsv));
				Ui_.TumblrLike_->setChecked (likes.contains (u"tumblr"_qsv));
				Ui_.SurfingbirdLike_->setChecked (likes.contains (u"surfingbird"_qsv));
				if (Ui_.VkontakteLike_->isChecked () &&
						Ui_.FacebookLike_->isChecked () &&
						Ui_.GoogleLike_->isChecked () &&
						Ui_.LiveJournalReward_->isChecked () &&
						Ui_.LiveJournalRepost_->isChecked () &&
						Ui_.TwitterLike_->isChecked () &&
						Ui_.TumblrLike_->isChecked () &&
						Ui_.SurfingbirdLike_->isChecked ())
					Ui_.AllLike_->setChecked (true);
			}
		}
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
		Ui_.Mood_->addItem (QString ());
		for (const auto& mood : profile->GetProfileData ().Moods_)
			Ui_.Mood_->addItem (mood.Name_, mood.Id_);

		Ui_.UserPic_->addItem (tr ("(default)"));
		const QString& path = Util::GetUserDir (Util::UserDir::Cache, "blogique/metida/avatars")
				.absoluteFilePath (Account_->GetAccountID ().toBase64 ().replace ('/', '_'));
		QPixmap pxm (path);
		Ui_.UserPicLabel_->setPixmap (pxm.scaled (pxm.width (), pxm.height ()));

		Ui_.UserPic_->addItems (profile->GetProfileData ().AvatarsID_);
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
		Ui_.ScreenComments_->addItem (tr ("Anonymous only"),
				CommentsManagement::ScreenAnonymouseComments);
		Ui_.ScreenComments_->addItem (tr ("Not from friends"),
				CommentsManagement::ShowFriendsComments);
		Ui_.ScreenComments_->addItem (tr ("Not from friends with links"),
				CommentsManagement::ScreenNotFromFriendsWithLinks);
		Ui_.ScreenComments_->addItem (tr ("Don't hide"),
				CommentsManagement::ShowComments);
		Ui_.ScreenComments_->addItem (tr ("All"),
				CommentsManagement::ScreenComments);

		Ui_.Adult_->addItem (tr ("Without adult content"),
				AdultContent::WithoutAdultContent);
		Ui_.Adult_->addItem (tr ("For adults (>14)"), AdultContent::AdultsFrom14);
		Ui_.Adult_->addItem (tr ("For adults (>18)"), AdultContent::AdultsFrom18);

		if (XmlSettingsManager::Instance ().Property ("SaveSelectedButtons", true).toBool ())
		{
			const auto& likes = XmlSettingsManager::Instance ()
					.Property ("SavedLikeButtons", QStringList ()).toStringList ();
			Ui_.VkontakteLike_->setChecked (likes.contains ("vkontakte"));
			Ui_.FacebookLike_->setChecked (likes.contains ("facebook"));
			Ui_.GoogleLike_->setChecked (likes.contains ("google"));
			Ui_.LiveJournalReward_->setChecked (likes.contains ("livejournal"));
			Ui_.LiveJournalRepost_->setChecked (likes.contains ("repost"));
			Ui_.TwitterLike_->setChecked (likes.contains ("twitter"));
			Ui_.TumblrLike_->setChecked (likes.contains ("tumblr"));
			Ui_.SurfingbirdLike_->setChecked (likes.contains ("surfingbird"));
		}
	}

	namespace
	{
		QObject* GetFirstICurrentSongKeeperInstance (const ICoreProxy_ptr& proxy)
		{
			auto plugins = proxy->GetPluginsManager ()->GetAllCastableRoots<Media::ICurrentSongKeeper*> ();
			return plugins.value (0);
		}
	}

	void PostOptionsWidget::handleAutoUpdateCurrentMusic ()
	{
		auto obj = GetFirstICurrentSongKeeperInstance (Proxy_);
		if (XmlSettingsManager::Instance ().Property ("AutoUpdateCurrentMusic", false).toBool () &&
				obj)
			connect (obj,
					SIGNAL (currentSongChanged (Media::AudioInfo)),
					this,
					SLOT (handleCurrentSongChanged (Media::AudioInfo)),
					Qt::UniqueConnection);
	}

	void PostOptionsWidget::on_Access__activated (int index)
	{
		if (static_cast<Access> (Ui_.Access_->itemData (index).toInt ()) == Access::Custom)
		{
			SelectGroupsDialog dlg (qobject_cast<LJProfile*> (Account_->GetProfile ()),
					AllowMask_);
			dlg.SetHeaderLabel (tr ("Choose friends groups that will be allowed to comment this post:"));

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
		const auto& dir = Util::GetUserDir (Util::UserDir::Cache, "blogique/metida/avatars");
		QString path = index ?
			dir.absoluteFilePath ((Account_->GetAccountID () +
					Ui_.UserPic_->itemText (index).toUtf8 ())
						.toBase64 ().replace ('/', '_')) :
			dir.absoluteFilePath ((Account_->GetAccountID ())
								.toBase64 ().replace ('/', '_'));
		QPixmap pxm (path);
		Ui_.UserPicLabel_->setPixmap (pxm.scaled (pxm.width (), pxm.height ()));
	}

	void PostOptionsWidget::on_AutoDetect__released ()
	{
		auto pluginObj = GetFirstICurrentSongKeeperInstance (Proxy_);
		if (!pluginObj)
			return;

		const auto& song = qobject_cast<Media::ICurrentSongKeeper*> (pluginObj)->GetCurrentSong ();
		Ui_.Music_->setText (QString ("\"%1\" by %2").arg (song.Title_)
				.arg (song.Artist_));
	}

	void PostOptionsWidget::handleCurrentSongChanged (const Media::AudioInfo& ai)
	{
		if (XmlSettingsManager::Instance ().Property ("AutoUpdateCurrentMusic", false).toBool ())
			Ui_.Music_->setText (QString ("\"%1\" by %2").arg (ai.Title_)
					.arg (ai.Artist_));
	}

	void PostOptionsWidget::handleHideMainOptions (bool)
	{
		Ui_.HideMainOptions_->setText (Ui_.HideMainOptions_->isChecked () ?
			tr ("Collapse") :
			tr ("Expand"));
		XmlSettingsManager::Instance ().setProperty ("CollapseMainOptions",
				!Ui_.HideMainOptions_->isChecked ());
	}

	void PostOptionsWidget::handleHideLikeButtons (bool)
	{
		Ui_.HideLikeButtons_->setText (Ui_.HideLikeButtons_->isChecked () ?
			tr ("Collapse") :
			tr ("Expand"));
		XmlSettingsManager::Instance ().setProperty ("CollapseLikeButtons",
				!Ui_.HideLikeButtons_->isChecked ());
	}

}
}
}
