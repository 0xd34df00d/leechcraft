/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "radiopilesmanager.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/iaudiopile.h>
#include <interfaces/media/iradiostationprovider.h>
#include "core.h"
#include "previewhandler.h"

namespace LeechCraft
{
namespace LMP
{
	RadioPilesManager::RadioPilesManager (const IPluginsManager *pm,
			PreviewHandler *handler, QObject *parent)
	: QObject { parent }
	, PreviewHandler_ { handler }
	, PilesModel_ { new QStandardItemModel { this } }
	{
		FillModel (pm);
	}

	QAbstractItemModel* RadioPilesManager::GetModel () const
	{
		return PilesModel_;
	}

	namespace
	{
		void HandlePile (Media::IAudioPile *pile, PreviewHandler *previewHandler)
		{
			const auto& query = QInputDialog::getText (nullptr,
					RadioPilesManager::tr ("Audio search"),
					RadioPilesManager::tr ("Enter the string to search for:"));
			if (query.isEmpty ())
				return;

			Media::AudioSearchRequest req;
			req.FreeForm_ = query;

			const auto pending = pile->Search (req);
			previewHandler->HandlePending (pending);
		}
	}

	void RadioPilesManager::FillModel (const IPluginsManager *pm)
	{
		for (auto pileObj : pm->GetAllCastableRoots<Media::IAudioPile*> ())
		{
			auto pile = qobject_cast<Media::IAudioPile*> (pileObj);

			auto item = new QStandardItem (tr ("Search in %1")
					.arg (pile->GetServiceName ()));
			item->setIcon (pile->GetServiceIcon ());
			item->setEditable (false);

			const auto function = [pile, this] { HandlePile (pile, PreviewHandler_); };
			item->setData (QVariant::fromValue<Media::ActionFunctor_f> (function),
					Media::RadioItemRole::ActionFunctor);

			PilesModel_->appendRow (item);
		}
	}
}
}
