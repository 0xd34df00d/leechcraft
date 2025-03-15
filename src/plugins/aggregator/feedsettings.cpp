/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "feedsettings.h"
#include <QDesktopServices>
#include <util/tags/tagscompleter.h>
#include <util/tags/tagscompletionmodel.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "storagebackendmanager.h"
#include "storagebackend.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Aggregator
{
	namespace
	{
		void SetLabelLink (QLabel *label, QString link)
		{
			QString shortLink;
			label->setToolTip (link);
			if (link.size () >= 160)
				shortLink = link.left (78) + "..." + link.right (78);
			else
				shortLink = link;

			if (QUrl { link }.isValid ())
			{
				link.insert (0, "<a href=\"");
				link.append ("\">" + shortLink + "</a>");
				label->setText (link);
			}
			else
				label->setText (shortLink);
		}
	}

	FeedSettings::FeedSettings (const ChannelShort& channel, QWidget *parent)
	: QDialog { parent }
	, FeedId_ { channel.FeedID_ }
	, ChannelId_ { channel.ChannelID_ }
	{
		Ui_.setupUi (this);
		setWindowIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());

		new Util::TagsCompleter { Ui_.ChannelTags_ };
		new Util::TagsCompleter { Ui_.FeedTags_ };

		Ui_.ChannelTags_->AddSelector ();
		Ui_.FeedTags_->AddSelector ();

		connect (Ui_.ChannelLink_,
				&QLabel::linkActivated,
				[] (const QString& url)
				{
					if (XmlSettingsManager::Instance ().property ("AlwaysUseExternalBrowser").toBool ())
						QDesktopServices::openUrl ({ url });
					else
					{
						const auto& e = Util::MakeEntity (QUrl::fromUserInput (url),
								{},
								FromUserInitiated | OnlyHandle);
						GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
					}
				});
		connect (Ui_.UpdateFavicon_,
				&QPushButton::released,
				this,
				[this, channel] { emit faviconRequested (ChannelId_, channel.Link_); });

		const auto itm = GetProxyHolder ()->GetTagsManager ();

		Ui_.ChannelTags_->setText (itm->JoinIDs (channel.Tags_));

		const auto& storage = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		if (const auto feedTags = storage->GetFeedTags (channel.FeedID_))
			Ui_.FeedTags_->setText (itm->Join (itm->GetTags (*feedTags)));

		storage->GetFeedSettings (channel.FeedID_).transform ([&] (auto&& settings)
				{
					Ui_.UpdateInterval_->setValue (settings.UpdateTimeout_);
					Ui_.NumItems_->setValue (settings.NumItems_);
					Ui_.ItemAge_->setValue (settings.ItemAge_);
					Ui_.AutoDownloadEnclosures_->setChecked (settings.AutoDownloadEnclosures_);
					return Util::Void {};
				});

		const auto& fullChannel = storage->GetChannel (ChannelId_);

		SetLabelLink (Ui_.ChannelLink_, fullChannel.Link_);
		SetLabelLink (Ui_.FeedURL_, storage->GetFeed (fullChannel.FeedID_).URL_);

		Ui_.ChannelDescription_->setHtml (fullChannel.Description_);
		Ui_.ChannelAuthor_->setText (fullChannel.Author_);
		Ui_.FeedNumItems_->setText (QString::number (storage->GetTotalItemsCount (ChannelId_)));

		if (const auto& maybeImg = storage->GetChannelPixmap (ChannelId_))
		{
			auto img = *maybeImg;
			if (img.width () > 400 || img.height () > 300)
				img = img.scaled (400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			Ui_.ChannelImage_->setPixmap (QPixmap::fromImage (img));
		}
	}

	void FeedSettings::accept ()
	{
		const auto& storage = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		const auto itm = GetProxyHolder ()->GetTagsManager ();

		const auto& channelTags = Ui_.ChannelTags_->text ();
		storage->SetChannelTags (ChannelId_, itm->SplitToIDs (channelTags));

		auto feedTags = Ui_.FeedTags_->text ();
		if (feedTags.isEmpty ())
			feedTags = channelTags;
		storage->SetFeedTags (FeedId_, itm->SplitToIDs (feedTags));

		storage->SetFeedSettings ({
				FeedId_,
				Ui_.UpdateInterval_->value (),
				Ui_.NumItems_->value (),
				Ui_.ItemAge_->value (),
				Ui_.AutoDownloadEnclosures_->checkState () == Qt::Checked
			});

		QDialog::accept ();
	}
}
}
