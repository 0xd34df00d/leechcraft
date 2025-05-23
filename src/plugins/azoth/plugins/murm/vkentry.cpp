/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkentry.h"
#include <QStringList>
#include <QtDebug>
#include <QTimer>
#include <interfaces/azoth/iproxyobject.h>
#include <util/sys/resourceloader.h>
#include "xmlsettingsmanager.h"
#include "vkaccount.h"
#include "vkmessage.h"
#include "vkconnection.h"
#include "photofetcher.h"
#include "vcarddialog.h"
#include "groupsmanager.h"
#include "vkchatentry.h"
#include "georesolver.h"
#include "util.h"
#include "photourlstorage.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	VkEntry::VkEntry (const UserInfo& info, VkAccount *account)
	: EntryBase (account)
	, Info_ (info)
	, RemoteTypingTimer_ (new QTimer (this))
	, LocalTypingTimer_ (new QTimer (this))
	{
		RemoteTypingTimer_->setInterval (6000);
		connect (RemoteTypingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleTypingTimeout ()));

		LocalTypingTimer_->setInterval (5000);
		connect (LocalTypingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (sendTyping ()));

		auto gm = account->GetGroupsManager ();
		for (const auto& id : info.Lists_)
		{
			const auto& info = gm->GetListInfo (id);
			if (info.ID_ == id)
				Groups_ << info.Name_;
		}

		XmlSettingsManager::Instance ()
				.RegisterObject ("EntryNameFormat", this, "handleEntryNameFormat");

		CheckPhotoChange ();
	}

	void VkEntry::UpdateInfo (const UserInfo& info, bool spontaneous)
	{
		const bool updateStatus = info.IsOnline_ != Info_.IsOnline_;
		const bool updateName = info.FirstName_ != Info_.FirstName_ ||
				info.LastName_ != Info_.LastName_;

		Info_ = info;

		if (updateName)
			nameChanged (GetEntryName ());
		if (updateStatus)
		{
			emit statusChanged (GetStatus (""), "");
			emit availableVariantsChanged (Variants ());

			if (spontaneous)
			{
				auto msg = new VkMessage (false, IMessage::Direction::In, IMessage::Type::StatusMessage, this);
				const auto& entryName = GetEntryName ();
				msg->SetBody (info.IsOnline_ ?
						tr ("%1 is now on the site again").arg (entryName) :
						tr ("%1 has left the site").arg (entryName));
				Store (msg);
			}
		}

		emit vcardUpdated ();

		CheckPhotoChange ();
	}

	const UserInfo& VkEntry::GetInfo () const
	{
		return Info_;
	}

	void VkEntry::UpdateAppInfo (const AppInfo& info, const QImage& image)
	{
		if (Info_.AppInfo_ == info && image == AppImage_)
			return;

		Info_.AppInfo_ = info;
		AppImage_ = image;
		emit entryGenerallyChanged ();
	}

	void VkEntry::Send (VkMessage *msg)
	{
		Account_->Send (GetInfo ().ID_, VkConnection::Type::Dialog, msg);
	}

	void VkEntry::SetSelf ()
	{
		IsSelf_ = true;
		emit groupsChanged (Groups ());
	}

	void VkEntry::SetNonRoster ()
	{
		IsNonRoster_ = true;
		emit groupsChanged (Groups ());
	}

	bool VkEntry::IsNonRoster () const
	{
		return IsNonRoster_;
	}

	Util::DefaultScopeGuard VkEntry::RegisterIn (VkChatEntry *chat)
	{
		if (!Chats_.contains (chat))
		{
			Chats_ << chat;
			ReemitGroups ();
		}

		QPointer<VkEntry> safeThis { this };
		return Util::MakeScopeGuard ([chat, safeThis, this]
				{
					if (safeThis)
					{
						if (Chats_.removeAll (chat))
							ReemitGroups ();
					}
				}).EraseType ();
	}

	void VkEntry::ReemitGroups ()
	{
		emit groupsChanged (Groups ());
	}

	VkMessage* VkEntry::FindMessage (qulonglong id) const
	{
		const auto pos = std::find_if (Messages_.begin (), Messages_.end (),
				[id] (VkMessage *msg) { return msg->GetID () == id; });
		return pos == Messages_.end () ? nullptr : *pos;
	}

	namespace
	{
		void FixEmoji (QString& text)
		{
			const uint32_t knowns []
			{
				0xD83DDE0A, 0xD83DDE03, 0xD83DDE09, 0xD83DDE06,
				0xD83DDE1C, 0xD83DDE0B, 0xD83DDE0D, 0xD83DDE0E,
				0xD83DDE12, 0xD83DDE0F, 0xD83DDE14, 0xD83DDE22,
				0xD83DDE2D, 0xD83DDE29, 0xD83DDE28, 0xD83DDE10,
				0xD83DDE0C, 0xD83DDE20, 0xD83DDE21, 0xD83DDE07,
				0xD83DDE30, 0xD83DDE32, 0xD83DDE33, 0xD83DDE37,
				0xD83DDE1A, 0xD83DDE08, 0x2764,		0xD83DDC4D,
				0xD83DDC4E, 0x261D,		0x270C,		0xD83DDC4C,
				0x26BD,		0x26C5,		0xD83CDF1F, 0xD83CDF4C,
				0xD83CDF7A, 0xD83CDF7B, 0xD83CDF39, 0xD83CDF45,
				0xD83CDF52, 0xD83CDF81, 0xD83CDF82, 0xD83CDF84,
				0xD83CDFC1, 0xD83CDFC6, 0xD83DDC0E, 0xD83DDC0F,
				0xD83DDC1C, 0xD83DDC2B, 0xD83DDC2E, 0xD83DDC03,
				0xD83DDC3B, 0xD83DDC3C, 0xD83DDC05, 0xD83DDC13,
				0xD83DDC18, 0xD83DDC94, 0xD83DDCAD, 0xD83DDC36,
				0xD83DDC31, 0xD83DDC37, 0xD83DDC11, 0x23F3,
				0x26BE,		0x26C4,		0x2600,		0xD83CDF3A,
				0xD83CDF3B, 0xD83CDF3C, 0xD83CDF3D, 0xD83CDF4A,
				0xD83CDF4B, 0xD83CDF4D, 0xD83CDF4E, 0xD83CDF4F,
				0xD83CDF6D, 0xD83CDF37, 0xD83CDF38, 0xD83CDF46,
				0xD83CDF49, 0xD83CDF50, 0xD83CDF51, 0xD83CDF53,
				0xD83CDF54, 0xD83CDF55, 0xD83CDF56, 0xD83CDF57,
				0xD83CDF69, 0xD83CDF83, 0xD83CDFAA, 0xD83CDFB1,
				0xD83CDFB2, 0xD83CDFB7, 0xD83CDFB8, 0xD83CDFBE,
				0xD83CDFC0, 0xD83CDFE6, 0xD83DDC00, 0xD83DDC0C,
				0xD83DDC1B, 0xD83DDC1D, 0xD83DDC1F, 0xD83DDC2A,
				0xD83DDC2C, 0xD83DDC2D, 0xD83DDC3A, 0xD83DDC3D,
				0xD83DDC2F, 0xD83DDC5C, 0xD83DDC7B, 0xD83DDC14,
				0xD83DDC23, 0xD83DDC24, 0xD83DDC40, 0xD83DDC42,
				0xD83DDC43, 0xD83DDC46, 0xD83DDC47, 0xD83DDC48,
				0xD83DDC51, 0xD83DDC60, 0xD83DDCA1, 0xD83DDCA3,
				0xD83DDCAA, 0xD83DDCAC, 0xD83DDD14, 0xD83DDD25,
				0xD83DDE04, 0xD83DDE02, 0xD83DDE15, 0xD83DDE2F,
				0xD83DDE26, 0xD83DDE35, 0xD83DDE1D, 0xD83DDE34,
				0xD83DDE18, 0xD83DDE1F, 0xD83DDE2C, 0xD83DDE36,
				0xD83DDE2A, 0xD83DDE2B, 0x263A,		0xD83DDE00,
				0xD83DDE25, 0xD83DDE1B, 0xD83DDE16, 0xD83DDE24,
				0xD83DDE23, 0xD83DDE27, 0xD83DDE11, 0xD83DDE05,
				0xD83DDE2E, 0xD83DDE1E, 0xD83DDE19, 0xD83DDE13,
				0xD83DDE01, 0xD83DDE31, 0xD83DDC7F, 0xD83DDC7D,
				0xD83DDC4F, 0xD83DDC4A, 0x270B,		0xD83DDE4F,
				0xD83DDC8B, 0xD83DDCA9, 0x2744,		0xD83CDF77,
				0xD83CDF78, 0xD83CDF85, 0xD83DDCA6, 0xD83DDC7A,
				0xD83DDC28, 0xD83DDD1E, 0xD83DDC79, 0xD83DDE38,
				0xD83DDE39, 0xD83DDE3C, 0xD83DDE3D, 0xD83DDE3E,
				0xD83DDE3F, 0xD83DDE3B, 0x23F0,		0x2601,
				0x260E,		0x2615,		0x267B,		0x26A0,
				0x26A1,		0x26D4,		0x26EA,		0x26F3,
				0x26F5,		0x26FD,		0x2702,		0x2708,
				0x2709,		0x270A,		0x270F,		0x2712,
				0x2728,		0xD83CDC04, 0xD83CDCCF, 0xD83CDD98,
				0xD83CDF02, 0xD83CDF0D, 0xD83CDF1B, 0xD83CDF1D,
				0xD83CDF1E, 0xD83CDF30, 0xD83CDF31, 0xD83CDF32,
				0xD83CDF33, 0xD83CDF34, 0xD83CDF35, 0xD83CDF3E,
				0xD83CDF3F, 0xD83CDF40, 0xD83CDF41, 0xD83CDF42,
				0xD83CDF43, 0xD83CDF44, 0xD83CDF47, 0xD83CDF5A,
				0xD83CDF5B, 0xD83CDF5C, 0xD83CDF5D, 0xD83CDF5E,
				0xD83CDF5F, 0xD83CDF60, 0xD83CDF61, 0xD83CDF62,
				0xD83CDF63, 0xD83CDF64, 0xD83CDF65, 0xD83CDF66,
				0xD83CDF67, 0xD83CDF68, 0xD83CDF6A, 0xD83CDF6B,
				0xD83CDF6C, 0xD83CDF6E, 0xD83CDF6F, 0xD83CDF70,
				0xD83CDF71, 0xD83CDF72, 0xD83CDF73, 0xD83CDF74,
				0xD83CDF75, 0xD83CDF76, 0xD83CDF79, 0xD83CDF7C,
				0xD83CDF80, 0xD83CDF88, 0xD83CDF89, 0xD83CDF8A,
				0xD83CDF8B, 0xD83CDF8C, 0xD83CDF8D, 0xD83CDF8E,
				0xD83CDF8F, 0xD83CDF90, 0xD83CDF92, 0xD83CDF93,
				0xD83CDFA3, 0xD83CDFA4, 0xD83CDFA7, 0xD83CDFA8,
				0xD83CDFA9, 0xD83CDFB0, 0xD83CDFB3, 0xD83CDFB4,
				0xD83CDFB9, 0xD83CDFBA, 0xD83CDFBB, 0xD83CDFBD,
				0xD83CDFBF, 0xD83CDFC2, 0xD83CDFC3, 0xD83CDFC4,
				0xD83CDFC7, 0xD83CDFC8, 0xD83CDFC9, 0xD83CDFCA,
				0xD83DDC01, 0xD83DDC02, 0xD83DDC04, 0xD83DDC06,
				0xD83DDC07, 0xD83DDC08, 0xD83DDC09, 0xD83DDC0A,
				0xD83DDC0B, 0xD83DDC0D, 0xD83DDC10, 0xD83DDC12,
				0xD83DDC15, 0xD83DDC16, 0xD83DDC17, 0xD83DDC19,
				0xD83DDC1A, 0xD83DDC1E, 0xD83DDC20, 0xD83DDC21,
				0xD83DDC22, 0xD83DDC25, 0xD83DDC26, 0xD83DDC27,
				0xD83DDC29, 0xD83DDC30, 0xD83DDC32, 0xD83DDC33,
				0xD83DDC34, 0xD83DDC35, 0xD83DDC38, 0xD83DDC39,
				0xD83DDC3E, 0xD83DDC44, 0xD83DDC45, 0xD83DDC4B,
				0xD83DDC50, 0xD83DDC52, 0xD83DDC53, 0xD83DDC54,
				0xD83DDC55, 0xD83DDC56, 0xD83DDC57, 0xD83DDC58,
				0xD83DDC59, 0xD83DDC5A, 0xD83DDC5B, 0xD83DDC5D,
				0xD83DDC5E, 0xD83DDC5F, 0xD83DDC61, 0xD83DDC62,
				0xD83DDC63, 0xD83DDC66, 0xD83DDC67, 0xD83DDC68,
				0xD83DDC69, 0xD83DDC6A, 0xD83DDC6B, 0xD83DDC6C,
				0xD83DDC6D, 0xD83DDC6E, 0xD83DDC6F, 0xD83DDC70,
				0xD83DDC71, 0xD83DDC72, 0xD83DDC74, 0xD83DDC75,
				0xD83DDC76, 0xD83DDC77, 0xD83DDC78, 0xD83DDC7C,
				0xD83DDC7E, 0xD83DDC80, 0xD83DDC81, 0xD83DDC82,
				0xD83DDC83, 0xD83DDC85, 0xD83DDC86, 0xD83DDC87,
				0xD83DDC88, 0xD83DDC89, 0xD83DDC8A, 0xD83DDC8C,
				0xD83DDC8D, 0xD83DDC8E, 0xD83DDC8F, 0xD83DDC90,
				0xD83DDC91, 0xD83DDC92, 0xD83DDC93, 0xD83DDC95,
				0xD83DDC96, 0xD83DDC97, 0xD83DDC98, 0xD83DDC99,
				0xD83DDC9A, 0xD83DDC9B, 0xD83DDC9C, 0xD83DDC9D,
				0xD83DDC9E, 0xD83DDC9F, 0xD83DDCA5, 0xD83DDCA7,
				0xD83DDCA8, 0xD83DDCB0, 0xD83DDCB3, 0xD83DDCB8,
				0xD83DDCBA, 0xD83DDCBB, 0xD83DDCBC, 0xD83DDCBD,
				0xD83DDCBE, 0xD83DDCBF, 0xD83DDCC0, 0xD83DDCC4,
				0xD83DDCC5, 0xD83DDCC7, 0xD83DDCC8, 0xD83DDCC9,
				0xD83DDCCA, 0xD83DDCCB, 0xD83DDCCC, 0xD83DDCCD,
				0xD83DDCCE, 0xD83DDCD0, 0xD83DDCD1, 0xD83DDCD2,
				0xD83DDCD3, 0xD83DDCD4, 0xD83DDCD5, 0xD83DDCD6,
				0xD83DDCD7, 0xD83DDCD8, 0xD83DDCD9, 0xD83DDCDA,
				0xD83DDCDC, 0xD83DDCDD, 0xD83DDCDF, 0xD83DDCE0,
				0xD83DDCE1, 0xD83DDCE2, 0xD83DDCE6, 0xD83DDCED,
				0xD83DDCEE, 0xD83DDCEF, 0xD83DDCF0, 0xD83DDCF1,
				0xD83DDCF7, 0xD83DDCF7, 0xD83DDCF7, 0xD83DDCFB,
				0xD83DDCFC, 0xD83DDD06, 0xD83DDD0E, 0xD83DDD11,
				0xD83DDD16, 0xD83DDD26, 0xD83DDD27, 0xD83DDD28,
				0xD83DDD29, 0xD83DDD2A, 0xD83DDD2B, 0xD83DDD2C,
				0xD83DDD2D, 0xD83DDD2E, 0xD83DDD31, 0xD83DDDFF,
				0xD83DDE3A, 0xD83DDE45, 0xD83DDE46, 0xD83DDE47,
				0xD83DDE48, 0xD83DDE49, 0xD83DDE4A, 0xD83DDE4B,
				0xD83DDE4C, 0xD83DDE4E, 0xD83DDE80, 0xD83DDE81,
				0xD83DDE82, 0xD83DDE83, 0xD83DDE84, 0xD83DDE85,
				0xD83DDE86, 0xD83DDE87, 0xD83DDE88, 0xD83DDE8A,
				0xD83DDE8C, 0xD83DDE8D, 0xD83DDE8E, 0xD83DDE8F,
				0xD83DDE90, 0xD83DDE91, 0xD83DDE92, 0xD83DDE93,
				0xD83DDE94, 0xD83DDE95, 0xD83DDE96, 0xD83DDE97,
				0xD83DDE98, 0xD83DDE99, 0xD83DDE9A, 0xD83DDE9B,
				0xD83DDE9D, 0xD83DDE9E, 0xD83DDE9F, 0xD83DDEA0,
				0xD83DDEA1, 0xD83DDEA3, 0xD83DDEA4, 0xD83DDEA7,
				0xD83DDEA8, 0xD83DDEAA, 0xD83DDEAC, 0xD83DDEB4,
				0xD83DDEB5, 0xD83DDEB6, 0xD83DDEBD, 0xD83DDEBF,
				0xD83DDEC0
			};

			for (auto known : knowns)
			{
				QString pattern;
				if (known > 0xffff)
					pattern.append (static_cast<quint16> (known >> 16)).append (static_cast<quint16> (known & 0xffff));
				else
					pattern.append (static_cast<quint16> (known));

				text.replace (pattern,
						QString ("<img src='http://vk.com/images/emoji/%1.png'/>")
							.arg (QString::number (known, 16).toUpper ()));
			}
		}
	}

	void VkEntry::HandleMessage (MessageInfo info, const FullMessageInfo& fullInfo)
	{
		if (FindMessage (info.ID_))
			return;

		if (info.Flags_ & MessageFlag::Outbox)
		{
			for (int i = Messages_.size () - 1; i >= 0; --i)
			{
				auto msg = Messages_.at (i);
				if (msg->GetID () == static_cast<qulonglong> (-1) &&
						msg->GetDirection () == IMessage::Direction::Out &&
						msg->GetBody () == info.Text_)
					return;
			}
		}

		const auto dir = info.Flags_ & MessageFlag::Outbox ?
				IMessage::Direction::Out :
				IMessage::Direction::In;

		if (dir == IMessage::Direction::In)
		{
			emit chatPartStateChanged (CPSActive, "");
			RemoteTypingTimer_->stop ();
			HasUnread_ = true;
		}

		if (info.Params_.remove ("emoji"))
			FixEmoji (info.Text_);

		auto msg = new VkMessage (false, dir, IMessage::Type::ChatMessage, this);
		msg->SetDateTime (info.TS_);
		msg->SetID (info.ID_);

		HandleAttaches (msg, info, fullInfo);

		Store (msg);
	}

	void VkEntry::HandleTypingNotification ()
	{
		emit chatPartStateChanged (CPSComposing, "");
		RemoteTypingTimer_->start ();
	}

	ICLEntry::Features VkEntry::GetEntryFeatures () const
	{
		Features result = FSupportsGrouping;
		result |= (IsNonRoster_ ? FSessionEntry : FPermanentEntry);

		if (IsSelf_)
			result |= FSelfContact;

		return result;
	}

	ICLEntry::EntryType VkEntry::GetEntryType () const
	{
		return ICLEntry::EntryType::Chat;
	}

	QString VkEntry::GetEntryName () const
	{
		return FormatUserInfoName (Info_);
	}

	void VkEntry::SetEntryName (const QString&)
	{
	}

	QString VkEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + QString::number (Info_.ID_);
	}

	QString VkEntry::GetHumanReadableID () const
	{
		return QString::number (Info_.ID_);
	}

	QStringList VkEntry::Groups () const
	{
		if (IsSelf_)
			return { tr ("Self contact") };

		if (IsNonRoster_)
			return { tr ("Non-friends") };

		auto result = Groups_;
		if (!Chats_.isEmpty ())
		{
			for (auto chat : Chats_)
				result << chat->GetGroupName ();
			result.removeDuplicates ();
		}
		return result;
	}

	void VkEntry::SetGroups (const QStringList& srcGroups)
	{
		if (IsSelf_ || IsNonRoster_)
			return;

		auto groups = srcGroups;

		if (!Chats_.isEmpty ())
			for (auto chat : Chats_)
				groups.removeAll (chat->GetGroupName ());

		Account_->GetGroupsManager ()->UpdateGroups (Groups_, groups, Info_.ID_);

		Groups_ = groups;
		emit groupsChanged (Groups_);
	}

	QStringList VkEntry::Variants () const
	{
		return Info_.IsOnline_ ? QStringList ("") : QStringList ();
	}

	void VkEntry::SetChatPartState (ChatPartState state, const QString&)
	{
		if (state == CPSComposing)
		{
			if (!LocalTypingTimer_->isActive ())
			{
				sendTyping ();
				LocalTypingTimer_->start ();
			}
		}
		else
			LocalTypingTimer_->stop ();
	}

	EntryStatus VkEntry::GetStatus (const QString&) const
	{
		return { Info_.IsOnline_ || IsSelf_ ? SOnline : SOffline, {} };
	}

	void VkEntry::ShowInfo ()
	{
		if (VCardDialog_)
			return;

		VCardDialog_ = new VCardDialog (this,
				Account_->GetParentProtocol ()->GetAzothProxy ()->GetAvatarsManager (),
				Account_->GetGeoResolver (),
				Account_->GetCoreProxy ());
		VCardDialog_->show ();
	}

	QList<QAction*> VkEntry::GetActions () const
	{
		return {};
	}

	namespace
	{
		QImage GetAppImage (const AppInfo& app, const QImage& appImage, VkAccount *acc)
		{
			QString name;
			if (app.Title_.isEmpty () && app.IsMobile_)
			{
				if (app.IsMobile_)
					name = "mobile";
				else
					return acc->GetParentProtocol ()->GetProtocolIcon ().pixmap (24, 24).toImage ();
			}
			else if (app.Title_ == "Windows Phone")
				name = "winphone";
			else if (app.Title_ == "Android" || app.Title_ == "iPhone" || app.Title_ == "iPad")
				name = app.Title_.toLower ();

			if (name.isEmpty ())
				return appImage;

			static const auto& loader = [] () -> const Util::ResourceLoader&
			{
				static Util::ResourceLoader loader { "azoth/murm/clients" };
				loader.AddGlobalPrefix ();
				loader.AddLocalPrefix ();
				return loader;
			} ();

			return loader.LoadPixmap (name).toImage ();
		}
	}

	QMap<QString, QVariant> VkEntry::GetClientInfo (const QString&) const
	{
		if (!Info_.IsOnline_)
			return {};

		QString name;

		const auto& app = Info_.AppInfo_;
		const auto& image = GetAppImage (app, AppImage_, Account_);
		if (!app.IsMobile_ && app.Title_.isEmpty ())
			name = tr ("Website");
		else if (app.Title_.isEmpty ())
			name = tr ("Mobile device");
		else
			name = app.Title_;

		return
		{
			{ "client_name", name },
			{ "client_image", image }
		};
	}

	void VkEntry::ChatTabClosed ()
	{
	}

	QVariant VkEntry::GetMetaInfo (DataField field) const
	{
		switch (field)
		{
		case DataField::BirthDate:
			return Info_.Birthday_;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown field"
				<< static_cast<int> (field);
		return {};
	}

	QList<QPair<QString, QVariant>> VkEntry::GetVCardRepresentation () const
	{
		return
		{
			{ tr ("First name"), Info_.FirstName_ },
			{ tr ("Last name"), Info_.LastName_ },
			{ tr ("Nick"), Info_.Nick_ },
			{ tr ("Photo"), Info_.BigPhoto_ },
			{ tr ("Birthday"), Info_.Birthday_ },
			{ tr ("Home phone"), Info_.HomePhone_ },
			{ tr ("Mobile phone"), Info_.MobilePhone_ },
			{ tr ("Timezone"), Info_.Timezone_ },
			{ tr ("City"), Account_->GetGeoResolver ()->GetCity (Info_.City_) },
			{ tr ("Country"), Account_->GetGeoResolver ()->GetCountry (Info_.Country_) },
		};
	}

	QFuture<QImage> VkEntry::RefreshAvatar (Size size)
	{
		const auto storage = Account_->GetPhotoStorage ();
		switch (size)
		{
		case Size::Thumbnail:
			return storage->GetImage (Info_.Photo_);
		case Size::Full:
			return storage->GetImage (Info_.BigPhoto_);
		}

		return {};
	}

	bool VkEntry::HasAvatar () const
	{
		return Info_.Photo_.isValid () ||
				Info_.BigPhoto_.isValid ();
	}

	bool VkEntry::SupportsSize (Size size) const
	{
		switch (size)
		{
		case Size::Thumbnail:
		case Size::Full:
			return true;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown size"
				<< static_cast<int> (size);

		return false;
	}

	void VkEntry::handleTypingTimeout ()
	{
		emit chatPartStateChanged (CPSPaused, "");
	}

	void VkEntry::sendTyping ()
	{
		Account_->GetConnection ()->SendTyping (Info_.ID_);
	}

	void VkEntry::handleEntryNameFormat ()
	{
		emit nameChanged (GetEntryName ());
	}

	void VkEntry::CheckPhotoChange ()
	{
		QPointer<QObject> safeThis { this };

		const auto id = Info_.ID_;
		const auto url = Info_.BigPhoto_.isValid () ?
				Info_.BigPhoto_ :
				Info_.Photo_;
		if (!url.isValid ())
			return;

		const auto photoUrlStorage = Account_->GetParentProtocol ()->GetPhotoUrlStorage ();

		QTimer::singleShot (0, this,
				[=, this]
				{
					if (!safeThis)
						return;

					const auto& storedUrl = photoUrlStorage->GetUserUrl (id);
					if (!storedUrl)
					{
						photoUrlStorage->SetUserUrl (id, url);
						return;
					}

					if (*storedUrl == url)
						return;

					qDebug () << Q_FUNC_INFO
							<< "photo for"
							<< id
							<< GetEntryName ()
							<< "changed from"
							<< *storedUrl
							<< "to"
							<< url;

					emit avatarChanged (this);

					photoUrlStorage->SetUserUrl (id, url);
				});
	}
}
}
}
