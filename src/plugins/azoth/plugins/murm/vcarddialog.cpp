/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcarddialog.h"
#include <QFuture>
#include <QtConcurrentRun>
#include <util/xpc/util.h>
#include <util/threads/futures.h>
#include <util/sll/functional.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include "structures.h"
#include "georesolver.h"
#include "vkentry.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	VCardDialog::VCardDialog (VkEntry *entry, IAvatarsManager *avatarsMgr,
			GeoResolver *geo, ICoreProxy_ptr proxy, QWidget *parent)
	: QDialog (parent)
	, Proxy_ (proxy)
	, Info_ (entry->GetInfo ())
	{
		Ui_.setupUi (this);
		setAttribute (Qt::WA_DeleteOnClose);

		Ui_.FirstName_->setText (Info_.FirstName_);
		Ui_.LastName_->setText (Info_.LastName_);
		Ui_.Nickname_->setText (Info_.Nick_);

		Ui_.Birthday_->setDate (Info_.Birthday_);
		Ui_.Birthday_->setDisplayFormat (Info_.Birthday_.year () != 1800 ? "dd MMMM yyyy" : "dd MMMM");

		if (Info_.Gender_)
			Ui_.Gender_->setText (Info_.Gender_ == 1 ? tr ("female") : tr ("male"));

		Ui_.HomePhone_->setText (Info_.HomePhone_);
		Ui_.MobilePhone_->setText (Info_.MobilePhone_);

		auto timezoneText = QString::number (Info_.Timezone_) + " GMT";
		if (Info_.Timezone_ > 0)
			timezoneText.prepend ('+');
		Ui_.Timezone_->setText (timezoneText);

		QPointer<VCardDialog> safeThis (this);

		if (Info_.Country_ > 0)
			Util::Sequence (this, geo->RequestCountry (Info_.Country_)) >>
					Util::BindMemFn (&QLineEdit::setText, Ui_.Country_);
		if (Info_.City_ > 0)
			Util::Sequence (this, geo->RequestCity (Info_.City_)) >>
					Util::BindMemFn (&QLineEdit::setText, Ui_.City_);

		if (!Info_.BigPhoto_.isValid ())
			return;

		Util::Sequence (this, avatarsMgr->GetAvatar (entry, IHaveAvatars::Size::Full)) >>
				[this] (const QImage& image)
				{
					return QtConcurrent::run ([this, image]
							{
								return image.scaled (Ui_.PhotoLabel_->size (),
										Qt::KeepAspectRatio,
										Qt::SmoothTransformation);
							});
				} >>
				[this] (const QImage& image)
				{
					Ui_.PhotoLabel_->setPixmap (QPixmap::fromImage (image));
				};
	}

	void VCardDialog::on_OpenVKPage__released ()
	{
		const auto& pageUrlStr = "http://vk.com/id" + QString::number (Info_.ID_);

		const auto& e = Util::MakeEntity (QUrl (pageUrlStr),
				QString (), FromUserInitiated | OnlyHandle);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
}
