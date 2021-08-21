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
#include <util/sll/functor.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
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

	FeedSettings::FeedSettings (const QModelIndex& mapped, const ICoreProxy_ptr& proxy, QWidget *parent)
	: QDialog { parent }
	, Index_ { mapped }
	, Proxy_ { proxy }
	{
		Ui_.setupUi (this);

		new Util::TagsCompleter { Ui_.ChannelTags_ };
		new Util::TagsCompleter { Ui_.FeedTags_ };

		Ui_.ChannelTags_->AddSelector ();
		Ui_.FeedTags_->AddSelector ();

		connect (Ui_.ChannelLink_,
				&QLabel::linkActivated,
				[this] (const QString& url)
				{
					if (XmlSettingsManager::Instance ()->property ("AlwaysUseExternalBrowser").toBool ())
						QDesktopServices::openUrl ({ url });
					else
					{
						const auto& e = Util::MakeEntity (QUrl::fromUserInput (url),
								{},
								FromUserInitiated | OnlyHandle);
						Proxy_->GetEntityManager ()->HandleEntity (e);
					}
				});
		connect (Ui_.UpdateFavicon_,
				&QPushButton::released,
				this,
				[this]
				{
					emit faviconRequested (Index_.data (ChannelRoles::ChannelID).value<IDType_t> (),
							Index_.data (ChannelRoles::ChannelLink).toString ());
				});

		const auto itm = proxy->GetTagsManager ();

		Ui_.ChannelTags_->setText (itm->Join (Index_.data (ChannelRoles::HumanReadableTags).toStringList ()));

		const auto& storage = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		const auto feedId = Index_.data (ChannelRoles::FeedID).value<IDType_t> ();

		if (const auto feedTags = storage->GetFeedTags (feedId))
			Ui_.FeedTags_->setText (itm->Join (itm->GetTags (*feedTags)));

		using Util::operator*;
		[&] (auto&& settings)
		{
			Ui_.UpdateInterval_->setValue (settings.UpdateTimeout_);
			Ui_.NumItems_->setValue (settings.NumItems_);
			Ui_.ItemAge_->setValue (settings.ItemAge_);
			Ui_.AutoDownloadEnclosures_->setChecked (settings.AutoDownloadEnclosures_);
		} * storage->GetFeedSettings (feedId);

		const auto cid = Index_.data (ChannelRoles::ChannelID).value<IDType_t> ();
		const auto& fullChannel = storage->GetChannel (cid);

		SetLabelLink (Ui_.ChannelLink_, fullChannel.Link_);
		SetLabelLink (Ui_.FeedURL_, storage->GetFeed (fullChannel.FeedID_).URL_);

		Ui_.ChannelDescription_->setHtml (fullChannel.Description_);
		Ui_.ChannelAuthor_->setText (fullChannel.Author_);
		Ui_.FeedNumItems_->setText (QString::number (storage->GetTotalItemsCount (cid)));

		if (const auto& maybeImg = storage->GetChannelPixmap (cid))
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

		const auto itm = Proxy_->GetTagsManager ();

		const auto& channelTags = Ui_.ChannelTags_->text ();
		storage->SetChannelTags (Index_.data (ChannelRoles::ChannelID).value<IDType_t> (), itm->SplitToIDs (channelTags));

		auto feedTags = Ui_.FeedTags_->text ();
		if (feedTags.isEmpty ())
			feedTags = channelTags;
		storage->SetFeedTags (Index_.data (ChannelRoles::FeedID).value<IDType_t> (), itm->SplitToIDs (feedTags));

		storage->SetFeedSettings ({
				Index_.data (ChannelRoles::FeedID).value<IDType_t> (),
				Ui_.UpdateInterval_->value (),
				Ui_.NumItems_->value (),
				Ui_.ItemAge_->value (),
				Ui_.AutoDownloadEnclosures_->checkState () == Qt::Checked
			});

		QDialog::accept ();
	}
}
}
