/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dnd.h"
#include <algorithm>
#include <utility>
#include <variant>
#include <QDataStream>
#include <QImageReader>
#include <QIODevice>
#include <QMimeData>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <interfaces/idatafilter.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/core/ientitymanager.h>
#include <util/compat/functional.h>
#include <util/gui/asyncdialogs.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include <util/sll/visitor.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include "chattabsmanager.h"
#include "util/azoth/entryidops.h"
#include "util/azoth/util.h"
#include "interfaces/azoth/iclentry.h"
#include "components/managers/entriesmanager.h"
#include "components/util/entries.h"
#include "components/util/misc.h"

namespace LC::Azoth::DndUtil
{
	const QString CLEntryFormat = "x-leechcraft/azoth-cl-entry"_qs;

	QString GetFormatId ()
	{
		return CLEntryFormat;
	}

	void Encode (const QList<MimeContactInfo>& entries, QMimeData& data)
	{
		QByteArray encoded;
		QDataStream stream (&encoded, QIODevice::WriteOnly);
		QStringList names;
		QList<QUrl> urls;
		for (const auto& info : entries)
		{
			Serialize (stream, info.Entry_->GetGlobalStrongestId ());
			stream << info.SourceGroup_.has_value () << info.SourceGroup_.value_or ({});

			names << FormatNameWithAddress (*info.Entry_);
			if (const auto url = info.Entry_->GetEntryUrl ())
				urls << *url;
		}

		data.setData (CLEntryFormat, encoded);
		data.setText (names.join ("; "_qs));
		if (!urls.isEmpty ())
			data.setUrls (urls);
	}

	QList<Util::TrackingPointer<ICLEntry>> DecodeEntryObjs (const EntriesManager& entries, const QMimeData *mime)
	{
		QList<Util::TrackingPointer<ICLEntry>> result;
		for (const auto& info : DecodeMimeInfos (entries, mime))
			if (!result.contains (info.Entry_))
				result << info.Entry_;
		return result;
	}

	QList<MimeContactInfo> DecodeMimeInfos (const EntriesManager& entries, const QMimeData *mime)
	{
		QList<MimeContactInfo> result;

		QDataStream stream (mime->data (CLEntryFormat));
		while (!stream.atEnd ())
		{
			const auto id = Deserialize<GlobalStrongestId> (stream);
			if (!id)
				break;

			bool hasGroup = false;
			QString group;
			stream >> hasGroup >> group;
			if (const auto entry = entries.GetEntryOrNull (id))
				result.append ({ entry, hasGroup ? std::optional { group } : std::nullopt });
		}

		return result;
	}

	bool HasContacts (const QMimeData *data)
	{
		return data->hasFormat (CLEntryFormat);
	}

	bool CanDropContacts (const EntriesManager& entries, const QMimeData& mime, ICLEntry& target)
	{
		return std::ranges::any_of (DecodeEntryObjs (entries, &mime),
				[&] (const Util::TrackingPointer<ICLEntry>& entry)
				{
					return std::holds_alternative<InvitePlan> (CheckInvitation (*entry, target));
				});
	}

	namespace
	{
		struct Trs
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Azoth)
		};

		auto ImageDropExtractor (const QMimeData& mime, auto defVal, auto imageMimeHandler, auto localFileHandler)
		{
			const auto& urls = mime.urls ();
			if (urls.size () > 1)
				return defVal;

			if (mime.hasImage ())
				return imageMimeHandler ();

			if (urls.isEmpty ())
				return defVal;

			const auto& url = urls [0];
			if (!url.isLocalFile ())
				return defVal;

			return localFileHandler (url.toLocalFile ());
		}

		bool IsImageDrop (const QMimeData& mime)
		{
			return ImageDropExtractor (mime, false,
					[] { return true; },
					[] (const QString& path) { return QImageReader { path }.canRead (); });
		}

		Util::Task<std::optional<QString>> GetInviteMessage (QWidget *parent, const QString& label)
		{
			return Util::Dialogs::GetText ({
						.Parent_ = parent,
						.Title_ = Trs::tr ("Conference invitation"),
						.Label_ = label + '\n' + Trs::tr ("You can add an optional invitation message:"),
					});
		}

		QString MakeInviteMessageLabel (const QList<InvitePlan>& plans)
		{
			const auto& first = plans [0];
			if (plans.size () == 1)
				return Trs::tr ("Do you want to invite %1 to %2?")
						.arg (first.InviteeLabel_, FormatNameWithAddress (first.Muc_->AsCLEntry ()));

			// by construction, all invites are either of the same entry to multiple rooms,
			// or of multiple entries to the same room
			enum class MultiInviteType : bool
			{
				SameEntryToManyRooms,
				ManyEntriesToSameRoom
			};

			const auto inviteType = plans [0].Muc_ == plans [1].Muc_ ?
					MultiInviteType::ManyEntriesToSameRoom :
					MultiInviteType::SameEntryToManyRooms;
			switch (inviteType)
			{
			case MultiInviteType::SameEntryToManyRooms:
				return Trs::tr ("Do you want to invite %1 to %n rooms?", nullptr, plans.size ())
						.arg (first.InviteeLabel_);
			case MultiInviteType::ManyEntriesToSameRoom:
				return Trs::tr ("Do you want to invite %n entries to %1?", nullptr, plans.size ())
						.arg (FormatNameWithAddress (first.Muc_->AsCLEntry ()));
			}
			std::unreachable ();
		}

		Util::Task<void> TryInvite (QList<Util::TrackingPointer<ICLEntry>> entries,
				Util::TrackingPointer<ICLEntry> target,
				QPointer<QWidget> parent)
		{
			QList<InviteRefusal> refusals;
			QList<InvitePlan> plans;

			for (const auto& entry : entries)
			{
				if (!entry || !target)
					continue;

				Util::Visit (CheckInvitation (*entry, *target),
						[] (NotAnInvitation) {},
						[&] (const InviteRefusal& refusal) { refusals << refusal; },
						[&] (const InvitePlan& plan) { plans << plan; });
			}

			const auto warnGuard = Util::MakeScopeGuard ([&]
					{
						if (!refusals.isEmpty ())
							Util::Dialogs::Warning (parent,
									Trs::tr ("Conference invitation"),
									Util::Map (refusals, &InviteRefusal::Message_).join ('\n'));
					});

			if (plans.isEmpty ())
				co_return;

			if (const auto& msg = co_await GetInviteMessage (parent, MakeInviteMessageLabel (plans)))
				for (const auto& plan : plans)
				{
					if (!plan.Muc_)
						continue;

					Util::Visit (plan.Invitee_,
							[&] (const Util::TrackingPointer<ICLEntry>& entry)
							{
								if (entry)
									plan.Muc_->InviteToMUC (*entry, *msg);
							},
							[&] (const QString& address) { plan.Muc_->InviteToMUC (address, *msg); });
				}
		}

		bool DropContacts (const EntriesManager& entries,
				const QMimeData& mime,
				const Util::TrackingPointer<ICLEntry>& target,
				QWidget *parent)
		{
			if (!HasContacts (&mime))
				return false;

			TryInvite (DecodeEntryObjs (entries, &mime), target, parent);
			return true;
		}

		bool DropFiles (TransferJobManager& transfers,
				QWidget *parent, const QMimeData& mime, const Util::TrackingPointer<ICLEntry>& target)
		{
			if (IsImageDrop (mime))
				return false;

			return OfferURLs (transfers, target, mime.urls (), parent);
		}

		struct Action
		{
			QString Label_;
			std::copyable_function<void () const> Fun_;
		};

		void SendInChat (const QImage& image,
				const Util::TrackingPointer<ICLEntry>& target,
				const std::optional<QString>& variant)
		{
			const auto& richBody = "<img src='"_qs + Util::GetAsBase64Src (image) + "'/>"_qs;
			const auto& msg = Trs::tr ("This message contains inline image, enable rich text to view it.");
			SendMessage (*target, { .Variant_ = variant, .Body_ = msg, .RichTextBody_ = richBody });
		}

		void SendLink (const QUrl& url,
				const Util::TrackingPointer<ICLEntry>& target,
				const std::optional<QString>& variant)
		{
			SendMessage (*target, { .Variant_ = variant, .Body_ = url.toString (QUrl::FullyEncoded) });
		}

		QList<Action> CollectDataFilters (ChatTabsManager& tabs,
				const QImage& image, const Util::TrackingPointer<ICLEntry>& target)
		{
			const auto& imageVar = QVariant::fromValue (image);
			const auto& entity = Util::MakeEntity (imageVar,
					{},
					NoParameters,
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
							[&tabs, target] (const QVariant& var)
							{
								const auto& tab = tabs.OpenChat (*target, true);
								if (const auto& url = var.toUrl ();
									!url.isEmpty () && tab)
									tab->insertMessageText (url.toString (QUrl::FullyEncoded));
							});

					const auto& label = verb + ": "_qs + variant.Name_;

					actions << Action { label, [thisEnt, obj] { qobject_cast<IEntityHandler*> (obj)->Handle (thisEnt); } };
				}
			}

			return actions;
		}

		Util::Task<void> PerformChoice (QList<Action> actions, QWidget *parent)
		{
			const auto& labels = Util::Map (actions, &Action::Label_);
			const auto& choice = co_await Util::Dialogs::GetItem ({
						.Parent_ = parent,
						.Title_ = Trs::tr ("Send image"),
						.Label_ = Trs::tr ("How would you like to send the image?"),
						.Items_ = labels,
						.Editable_ = false,
					});
			const auto idx = choice ? labels.indexOf (*choice) : -1;
			if (idx < 0)
				co_return;

			actions [idx].Fun_ ();
		}

		bool DropImages (const DndContext& context,
				const QMimeData& mime, const Util::TrackingPointer<ICLEntry>& target)
		{
			if (!IsImageDrop (mime) || !target)
				return false;

			const auto image = ImageDropExtractor (mime, QImage {},
					[&] { return mime.imageData ().value<QImage> (); },
					[] (const QString& path) { return QImage { path }; });
			if (image.isNull ())
				return OfferURLs (context.Transfers_, target, mime.urls (), context.Parent_);

			const auto variant = context.Tabs_.GetSelectedVariant (&*target);
			const QPointer parent { context.Parent_ };

			auto actions = CollectDataFilters (context.Tabs_, image, target);
			actions << Action { Trs::tr ("Send directly in chat"), [=] { SendInChat (image, target, variant); } };
			if (const auto& url = mime.urls ().value (0);
				url.isValid ())
			{
				if (url.scheme () != "file")
					actions << Action { Trs::tr ("Send link"), [=] { SendLink (url, target, variant); } };
				else
					actions << Action { Trs::tr ("Send as file"), [=, &transfers = context.Transfers_] { OfferURLs (transfers, target, { url }, parent); } };
			}
			PerformChoice (std::move (actions), context.Parent_);
			return true;
		}
	}

	bool CanDropAttachments (const QMimeData& mime)
	{
		return IsImageDrop (mime) || CouldOfferURLs (mime.urls ());
	}

	bool DropAttachments (const QMimeData& mime, const Util::TrackingPointer<ICLEntry>& target, const DndContext& context)
	{
		const auto droppedFiles = DropFiles (context.Transfers_, context.Parent_, mime, target);
		const auto droppedImages = DropImages (context, mime, target);
		return droppedFiles || droppedImages;
	}

	bool CanHandleDroppedOnto (const EntriesManager& entries, const QMimeData& mime, ICLEntry& target)
	{
		return CanDropContacts (entries, mime, target) || CanDropAttachments (mime);
	}

	bool HandleDroppedOnto (const QMimeData& mime, const Util::TrackingPointer<ICLEntry>& target, const DndContext& context)
	{
		if (!target)
			return false;

		const auto droppedContacts = DropContacts (context.Entries_, mime, target, context.Parent_);
		const auto droppedAttachments = DropAttachments (mime, target, context);
		return droppedContacts || droppedAttachments;
	}
}
