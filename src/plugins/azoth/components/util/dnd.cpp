/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dnd.h"
#include <QDataStream>
#include <QInputDialog>
#include <QImageReader>
#include <QIODevice>
#include <QMimeData>
#include <QString>
#include <QUrl>
#include <interfaces/idatafilter.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include "chattabsmanager.h"
#include "util/azoth/entryidops.h"
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

	QByteArray Encode (const QList<MimeContactInfo>& entries, QMimeData *data)
	{
		QByteArray encoded;
		QDataStream stream (&encoded, QIODevice::WriteOnly);
		for (const auto& info : entries)
		{
			Serialize (stream, info.Entry_->GetGlobalStrongestId ());
			stream << info.Group_;
		}

		if (data)
			data->setData (CLEntryFormat, encoded);

		return encoded;
	}

	ICLEntry* DecodeEntryObj (const EntriesManager& entries, const QMimeData *mime)
	{
		QDataStream stream (mime->data (CLEntryFormat));
		const auto id = Deserialize<GlobalStrongestId> (stream);
		return entries.GetEntryOrNull (id);
	}

	QList<Util::TrackingPointer<ICLEntry>> DecodeEntryObjs (const EntriesManager& entries, const QMimeData *mime)
	{
		return Util::Map (DecodeMimeInfos (entries, mime), &MimeContactInfo::Entry_);
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

			QString group;
			stream >> group;
			if (const auto entry = entries.GetEntryOrNull (id))
				result.append ({ entry, group });
		}

		return result;
	}

	bool HasContacts (const QMimeData *data)
	{
		return data->hasFormat (CLEntryFormat);
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

		void PerformChoice (const QList<Action>& actions, QWidget *parent)
		{
			bool ok = false;
			const auto& labels = Util::Map (actions, &Action::Label_);
			const auto& choice = QInputDialog::getItem (parent,
					Trs::tr ("Send image"),
					Trs::tr ("How would you like to send the image?"),
					labels,
					0,
					false,
					&ok);
			const auto idx = labels.indexOf (choice);
			if (!ok || idx < 0)
				return;

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

			const auto variant = context.Tabs_.GetActiveVariant (&*target);

			auto actions = CollectDataFilters (context.Tabs_, image, target);
			actions << Action { Trs::tr ("Send directly in chat"), [&] { SendInChat (image, target, variant); } };
			if (const auto& url = mime.urls ().value (0);
				url.isValid ())
			{
				if (url.scheme () != "file")
					actions << Action { Trs::tr ("Send link"), [&, url] { SendLink (url, target, variant); } };
				else
					actions << Action { Trs::tr ("Send as file"), [&, url] { OfferURLs (context.Transfers_, target, { url }); }};
			}
			PerformChoice (actions, context.Parent_);
			return true;
		}
	}

	bool HandleDroppedOnto (const QMimeData& mime, const Util::TrackingPointer<ICLEntry>& target, const DndContext& context)
	{
		if (!target)
			return false;

		const auto droppedContacts = DropContacts (context.Entries_, mime, target, context.Parent_);
		const auto droppedFiles = DropFiles (context.Transfers_, context.Parent_, mime, target);
		const auto droppedImages = DropImages (context, mime, target);
		return droppedContacts || droppedFiles || droppedImages;
	}
}
