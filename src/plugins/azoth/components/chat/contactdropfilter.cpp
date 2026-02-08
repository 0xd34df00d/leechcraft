/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "contactdropfilter.h"
#include <QDropEvent>
#include <QFile>
#include <QImage>
#include <QInputDialog>
#include <QUrl>
#include <QMimeData>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/idatafilter.h>
#include <interfaces/ientityhandler.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/imucentry.h"
#include "components/transfers/transferjobmanager.h"
#include "components/util/dnd.h"
#include "components/util/misc.h"
#include "chattab.h"

namespace LC::Azoth
{
	ContactDropFilter::ContactDropFilter (TransferJobManager& tjm, ChatTab& tab)
	: QObject { &tab }
	, Transfers_ { tjm }
	, ChatTab_ { tab }
	{
	}

	bool ContactDropFilter::eventFilter (QObject*, QEvent *e)
	{
		if (e->type () != QEvent::Drop)
			return false;

		HandleDrop (static_cast<QDropEvent*> (e)->mimeData ());
		return true;
	}

	namespace
	{
		std::optional<QImage> ExtractLocalImageFile (const QList<QUrl>& urls)
		{
			if (urls.size () != 1)
				return {};

			const auto& local = urls.at (0).toLocalFile ();
			if (!QFile::exists (local))
				return {};

			QImage img { local };
			if (img.isNull ())
				return {};
			return img;
		}
	}

	void ContactDropFilter::HandleDrop (const QMimeData *data)
	{
		const auto& imgData = data->imageData ();

		const auto& urls = data->urls ();
		if (data->hasImage () && urls.size () <= 1)
			HandleImageDropped (imgData.value<QImage> (), urls.value (0));
		else if (const auto image = ExtractLocalImageFile (urls))
			HandleImageDropped (*image, urls.value (0));
		else if (DndUtil::HasContacts (data))
			HandleContactsDropped (data);
		else if (!urls.isEmpty ())
			OfferURLs (Transfers_, ChatTab_.GetEntry<ICLEntry> (), urls);
	}

	namespace
	{
		void SendInChat (const QImage& image, const ChatTab& chatTab)
		{
			const auto entry = chatTab.GetEntry<ICLEntry> ();
			if (!entry)
				return;

			const auto& richBody = "<img src='"_qs + Util::GetAsBase64Src (image) + "'/>"_qs;
			const auto& msg = ContactDropFilter::tr ("This message contains inline image, enable XHTML-IM to view it.");
			SendMessage (*entry, { .Variant_ = chatTab.GetSelectedVariant (), .Body_ = msg, .RichTextBody_ = richBody });
		}

		void SendLink (const QUrl& url, const ChatTab& chatTab)
		{
			if (const auto entry = chatTab.GetEntry<ICLEntry> ())
				SendMessage (*entry, { .Variant_ = chatTab.GetSelectedVariant (), .Body_ = url.toEncoded () });
		}

		struct Action
		{
			QString Label_;
			std::function<void ()> Fun_;
		};

		QList<Action> CollectDataFilters (const QImage& image, ChatTab& tab)
		{
			const auto& imageVar = QVariant::fromValue (image);
			const auto& entity = Util::MakeEntity (imageVar,
					{},
					TaskParameter::NoParameters,
					"x-leechcraft/data-filter-request"_qs);

			QList<Action> actions;

			for (const auto obj : GetProxyHolder ()->GetEntityManager ()->GetPossibleHandlers (entity))
			{
				const auto idf = qobject_cast<IDataFilter*> (obj);

				const auto& verb = idf->GetFilterVerb ();

				for (const auto& variant : idf->GetFilterVariants (imageVar))
				{
					auto thisEnt = entity;
					thisEnt.Additional_ ["DataFilter"_qs] = variant.Name_;
					thisEnt.Additional_ ["DataFilterCallback"_qs] = QVariant::fromValue<DataFilterCallback_f> (
							[&tab] (const QVariant& var)
							{
								if (const auto& url = var.toUrl ();
									!url.isEmpty ())
									tab.insertMessageText (url.toEncoded ());
							});

					const auto& label = verb + ": "_qs + variant.Name_;

					actions << Action { label, [thisEnt, obj] { qobject_cast<IEntityHandler*> (obj)->Handle (thisEnt); } };
				}
			}

			return actions;
		}

		void PerformChoice (const QList<Action>& actions, ChatTab& chatTab)
		{
			bool ok = false;
			const auto& labels = Util::Map (actions, &Action::Label_);
			const auto& choice = QInputDialog::getItem (&chatTab,
					ContactDropFilter::tr ("Send image"),
					ContactDropFilter::tr ("How would you like to send the image?"),
					labels,
					0,
					false,
					&ok);
			const auto idx = labels.indexOf (choice);
			if (!ok || idx < 0)
				return;

			actions [idx].Fun_ ();
		}
	}

	void ContactDropFilter::HandleImageDropped (const QImage& image, const QUrl& url)
	{
		auto actions = CollectDataFilters (image, ChatTab_);
		actions << Action { tr ("Send directly in chat"), [this, &image] { SendInChat (image, ChatTab_); } };

		if (url.scheme () != "file")
			actions << Action { tr ("Send link"), [this, url] { SendLink (url, ChatTab_); } };
		else
			actions << Action { tr ("Send as file"), [this, url] { OfferURLs (Transfers_, ChatTab_.GetEntry<ICLEntry> (), { url }); }};

		PerformChoice (actions, ChatTab_);
	}

	namespace
	{
		bool CanEntryBeInvited (ICLEntry *thisEntry, ICLEntry *entry)
		{
			const bool isMuc = thisEntry->GetEntryType () == ICLEntry::EntryType::MUC;

			const auto entryAcc = entry->GetParentAccount ();
			const auto thisAcc = thisEntry->GetParentAccount ();
			if (thisAcc->GetParentProtocol () != entryAcc->GetParentProtocol ())
				return false;

			const bool isThatMuc = entry->GetEntryType () == ICLEntry::EntryType::MUC;
			return isThatMuc != isMuc;
		}
	}

	void ContactDropFilter::HandleContactsDropped (const QMimeData *data)
	{
		const auto thisEntry = ChatTab_.GetEntry<ICLEntry> ();
		const bool isMuc = thisEntry->GetEntryType () == ICLEntry::EntryType::MUC;

		auto entries = DndUtil::DecodeEntryObjs (data);
		entries.removeIf ([thisEntry] (QObject *entryObj)
					{
						return !CanEntryBeInvited (thisEntry, qobject_cast<ICLEntry*> (entryObj));
					});

		if (entries.isEmpty ())
			return;

		QString text;
		if (entries.size () > 1)
		{
			const auto& entryName = thisEntry->GetEntryName ();
			text = isMuc ?
					tr ("Invitation message for %n contact(s) to %1:", nullptr, entries.size ()).arg (entryName) :
					tr ("Invitation message for %1 to %n conference(s):", nullptr, entries.size ()).arg (entryName);
		}
		else
		{
			const auto muc = isMuc ?
					thisEntry :
					qobject_cast<ICLEntry*> (entries.first ());
			const auto entry = isMuc ?
					qobject_cast<ICLEntry*> (entries.first ()) :
					thisEntry;
			text = tr ("Invitation message for %1 in %2:").arg (entry->GetEntryName (), muc->GetEntryName ());
		}

		bool ok = false;
		const auto reason = QInputDialog::getText (nullptr,
				tr ("Invite contacts"),
				text,
				QLineEdit::Normal,
				{},
				&ok);
		if (!ok)
			return;

		if (isMuc)
		{
			const auto muc = qobject_cast<IMUCEntry*> (thisEntry->GetQObject ());
			for (const auto& entry : entries)
				muc->InviteToMUC (qobject_cast<ICLEntry*> (entry)->GetHumanReadableID (), reason);
		}
		else
		{
			const auto thisId = thisEntry->GetHumanReadableID ();
			for (const auto& mucEntryObj : entries)
			{
				const auto muc = qobject_cast<IMUCEntry*> (mucEntryObj);
				muc->InviteToMUC (thisId, reason);
			}
		}
	}
}
