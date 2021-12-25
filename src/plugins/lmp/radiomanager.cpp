/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "radiomanager.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QTimer>
#include <QMimeData>
#include <QtDebug>
#include <util/models/dndactionsmixin.h>
#include <util/models/mergemodel.h>
#include <util/sll/containerconversions.h>
#include <util/sll/dropargs.h>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/gui/util.h>
#include <interfaces/media/iradiostationprovider.h>
#include <interfaces/media/imodifiableradiostation.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "player.h"
#include "xmlsettingsmanager.h"
#include "radiocustomstreams.h"
#include "radiopilesmanager.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		class RadioModel : public Util::DndActionsMixin<Util::MergeModel>
		{
			RadioManager * const Manager_;
		public:
			RadioModel (const QStringList& headers, RadioManager *manager)
			: DndActionsMixin { headers, manager }
			, Manager_ { manager }
			{
				setSupportedDragActions (Qt::CopyAction | Qt::MoveAction);
			}

			QStringList mimeTypes () const override
			{
				return
				{
					"text/uri-list",
					"x-leechcraft-lmp/media-info-list",
					"x-leechcraft-lmp/radio-ids"
				};
			}

			QMimeData* mimeData (const QModelIndexList& indexes) const override
			{
				QList<QUrl> urls;
				QList<MediaInfo> infos;

				QStringList stationsIds;

				for (const auto& index : indexes)
				{
					const auto type = index.data (Media::RadioItemRole::ItemType).toInt ();
					switch (static_cast<Media::RadioType> (type))
					{
					case Media::RadioType::Predefined:
					case Media::RadioType::CustomAddableStreams:
						stationsIds << index.data (Media::RadioItemRole::RadioID).toString ();
						break;
					case Media::RadioType::TracksList:
					case Media::RadioType::SingleTrack:
					case Media::RadioType::TracksRoot:
						for (const auto& info : Manager_->GetSources (index))
						{
							urls << info.Other_ ["URL"].toUrl ();
							infos << MediaInfo::FromAudioInfo (info);
						}
					default:
						break;
					}
				}

				urls.removeAll ({});

				if (urls.isEmpty () && stationsIds.isEmpty ())
					return nullptr;

				auto result = new QMimeData;
				result->setUrls (urls);

				Util::Save2MimeData (result, "x-leechcraft-lmp/media-info-list", infos);
				Util::Save2MimeData (result, "x-leechcraft-lmp/radio-ids", stationsIds);

				return result;
			}
		};
	}

	RadioManager::RadioManager (QObject *parent)
	: QObject { parent }
	, MergeModel_ { new RadioModel { { "Station" }, this } }
	, AutoRefreshTimer_ { new QTimer { this } }
	{
		XmlSettingsManager::Instance ().RegisterObject ({ "AutoRefreshRadios",
					"RadioRefreshTimeout" },
				this, "handleRefreshSettingsChanged");
		handleRefreshSettingsChanged ();

		connect (AutoRefreshTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (refreshAll ()));
	}

	void RadioManager::InitProviders ()
	{
		auto pm = GetProxyHolder ()->GetPluginsManager ();

		const auto rpm = new RadioPilesManager { pm, this };
		MergeModel_->AddModel (rpm->GetModel ());

		InitProvider (new RadioCustomStreams (this));

		auto providerObjs = pm->GetAllCastableRoots<Media::IRadioStationProvider*> ();
		for (auto provObj : providerObjs)
			InitProvider (provObj);
	}

	QAbstractItemModel* RadioManager::GetModel () const
	{
		return MergeModel_;
	}

	void RadioManager::Refresh (const QModelIndex& index)
	{
		if (!WithSourceProv (index,
					Util::DropArgs ([] { return true; }),
					Util::DropArgs ([] { return false; })))
			return;

		WithSourceProv (index,
				[] (Media::IRadioStationProvider *prov, const QModelIndex& srcIdx)
					{ prov->RefreshItems ({ srcIdx }); });
	}

	void RadioManager::AddUrl (const QModelIndex& index, const QUrl& url, const QString& name)
	{
		WithSourceProv (index,
				[url, name] (Media::IRadioStationProvider *prov, const QModelIndex& srcIdx)
				{
					const auto radio = prov->GetRadioStation (srcIdx, {});
					if (!radio)
					{
						qWarning () << Q_FUNC_INFO
								<< "got a null radio station from provider";
						return;
					}

					auto modifiable = qobject_cast<Media::IModifiableRadioStation*> (radio->GetQObject ());
					if (!modifiable)
					{
						qWarning () << Q_FUNC_INFO
								<< radio->GetRadioName ()
								<< "is not modifiable";
						return;
					}

					modifiable->AddItem (url, name);
				});
	}

	void RadioManager::RemoveUrl (const QModelIndex& index)
	{
		WithSourceProv (index,
				[] (Media::IRadioStationProvider *prov, const QModelIndex& index)
				{
					const auto radio = prov->GetRadioStation (index, {});
					if (!radio)
					{
						qWarning () << Q_FUNC_INFO
								<< "got a null radio station from provider";
						return;
					}

					auto modifiable = qobject_cast<Media::IModifiableRadioStation*> (radio->GetQObject ());
					if (!modifiable)
					{
						qWarning () << Q_FUNC_INFO
								<< radio->GetRadioName ()
								<< "is not modifiable";
						return;
					}

					modifiable->RemoveItem (index);
				});
	}

	void RadioManager::Handle (const QModelIndex& index, Player *player)
	{
		const auto& funcVar = index.data (Media::RadioItemRole::ActionFunctor);
		if (funcVar.isValid ())
			return Util::Visit (funcVar.value<Media::ActionFunctor_f> (),
			        [] (const std::function<void ()>& func) { return func (); },
			        [&index] (const std::function<void (QModelIndex)>& func) { func (index); });

		QString param;

		const auto rawType = index.data (Media::RadioItemRole::ItemType).toInt ();
		const auto type = static_cast<Media::RadioType> (rawType);
		switch (type)
		{
		case Media::RadioType::None:
			return;
		case Media::RadioType::Predefined:
		case Media::RadioType::CustomAddableStreams:
		case Media::RadioType::RadioAction:
			break;
		case Media::RadioType::SimilarArtists:
			param = QInputDialog::getText (0,
					tr ("Similar artists radio"),
					tr ("Enter artist name for which to tune the similar artists radio station:"));
			if (param.isEmpty ())
				return;
			break;
		case Media::RadioType::GlobalTag:
			param = QInputDialog::getText (0,
					tr ("Global tag radio"),
					tr ("Enter global tag name:"));
			if (param.isEmpty ())
				return;
			break;
		case Media::RadioType::TracksList:
		case Media::RadioType::SingleTrack:
		case Media::RadioType::TracksRoot:
		{
			QList<AudioSource> sources;
			for (const auto& info : GetSources (index))
			{
				const auto& url = info.Other_ ["URL"].toUrl ();
				player->PrepareURLInfo (url, MediaInfo::FromAudioInfo (info));
				sources << url;
			}
			player->Enqueue (sources, Player::EnqueueNone);
			return;
		}
		}

		WithSourceProv (index,
				[player, &param] (Media::IRadioStationProvider *prov, const QModelIndex& srcIdx)
				{
					if (auto station = prov->GetRadioStation (srcIdx, param))
						player->SetRadioStation (station);
				});
	}

	void RadioManager::HandleWokeUp ()
	{
		if (XmlSettingsManager::Instance ().property ("RefreshRadioOnWakeup").toBool ())
			QTimer::singleShot (15000,
					this,
					SLOT (refreshAll ()));
	}

	QList<Media::AudioInfo> RadioManager::GetSources (const QModelIndex& index) const
	{
		const auto intRadioType = index.data (Media::RadioItemRole::ItemType).toInt ();
		switch (static_cast<Media::RadioType> (intRadioType))
		{
		case Media::RadioType::TracksRoot:
		{
			QList<Media::AudioInfo> result;
			for (int i = 0, rc = index.model ()->rowCount (index); i < rc; ++i)
				result += GetSources (index.model ()->index (i, 0, index));
			return result;
		}
		case Media::RadioType::TracksList:
		case Media::RadioType::SingleTrack:
		{
			QList<Media::AudioInfo> result;

			const auto& infosVar = index.data (Media::RadioItemRole::TracksInfos);
			const auto& radioID = index.data (Media::RadioItemRole::RadioID).toString ();
			const auto& pluginID = index.data (Media::RadioItemRole::PluginID).toByteArray ();
			for (auto info : infosVar.value<QList<Media::AudioInfo>> ())
			{
				const auto& url = info.Other_ ["URL"].toUrl ();
				if (!url.isValid ())
				{
					qWarning () << Q_FUNC_INFO
							<< "ignoring invalid URL"
							<< info.Other_;
					continue;
				}

				if (!radioID.isEmpty () && !pluginID.isEmpty ())
				{
					info.Other_ ["LMP/RadioID"] = radioID;
					info.Other_ ["LMP/PluginID"] = pluginID;
				}

				result << info;
			}

			return result;
		}
		default:
			return {};
		}
	}

	QList<Media::AudioInfo> RadioManager::GetSources (const QList<QModelIndex>& indices) const
	{
		return Util::Concat (Util::Map (indices,
					[this] (const QModelIndex& idx) { return GetSources (idx); }));
	}

	Media::IRadioStation_ptr RadioManager::GetRadioStation (const QString& radioId) const
	{
		if (radioId.isEmpty ())
			return {};

		QList<QModelIndex> items { {} };
		for (int i = 0; i < items.size (); ++i)
		{
			const auto& index = items.at (i);

			if (index.data (Media::RadioItemRole::RadioID).toString () == radioId)
			{
				Media::IRadioStation_ptr station;
				WithSourceProv (index,
						[&station] (Media::IRadioStationProvider *prov, const QModelIndex& srcIdx)
							{ station = prov->GetRadioStation (srcIdx, {}); });
				return station;
			}

			for (int j = 0; j < MergeModel_->rowCount (index); ++j)
				items << MergeModel_->index (j, 0, index);
		}

		return {};
	}

	void RadioManager::InitProvider (QObject *provObj)
	{
		const auto prov = qobject_cast<Media::IRadioStationProvider*> (provObj);
		for (const auto model : prov->GetRadioListItems ())
		{
			MergeModel_->AddModel (model);
			Model2Prov_ [model] = prov;
		}
	}

	template<typename F>
	std::result_of_t<F (Media::IRadioStationProvider*, QModelIndex)>
		RadioManager::WithSourceProv (const QModelIndex& mapped, F f) const
	{
		return WithSourceProv (mapped,
				f,
				[&mapped] (const QModelIndex&)
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown provider for"
							<< mapped
							<< mapped.data ();
					return;
				});
	}

	template<typename Succ, typename Fail>
	std::result_of_t<Succ (Media::IRadioStationProvider*, QModelIndex)>
		RadioManager::WithSourceProv (const QModelIndex& mapped, Succ succ, Fail fail) const
	{
		const auto& src = MergeModel_->mapToSource (mapped);
		const auto prov = Model2Prov_.value (src.model ());
		if (!prov)
			return fail (src);

		return succ (prov, src);
	}

	void RadioManager::refreshAll ()
	{
		// TODO Qt 5.14 remove values() call
		for (auto prov : Util::AsSet (Model2Prov_.values ()))
			prov->RefreshItems ({});
	}

	void RadioManager::handleRefreshSettingsChanged ()
	{
		AutoRefreshTimer_->stop ();

		const auto interval = XmlSettingsManager::Instance ()
				.property ("RadioRefreshTimeout").toInt ();
		AutoRefreshTimer_->setInterval (interval * 60 * 60 * 1000);

		if (XmlSettingsManager::Instance ().property ("AutoRefreshRadios").toBool ())
			AutoRefreshTimer_->start ();
	}
}
}
