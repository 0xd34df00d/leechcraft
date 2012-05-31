/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "radiowidget.h"
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/iradiostationprovider.h>
#include "core.h"
#include "player.h"

namespace LeechCraft
{
namespace LMP
{
	RadioWidget::RadioWidget (QWidget *parent)
	: QWidget (parent)
	, Player_ (0)
	{
		Ui_.setupUi (this);

		Providers_ = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableTo<Media::IRadioStationProvider*> ();
		Q_FOREACH (auto provider, Providers_)
			Ui_.ProviderBox_->addItem (provider->GetRadioName ());

		Ui_.ProviderBox_->setCurrentIndex (-1);

		Ui_.StationType_->addItem (tr ("Similar"));
		Ui_.StationType_->addItem (tr ("Tag"));
	}

	void RadioWidget::SetPlayer (Player *player)
	{
		Player_ = player;
	}

	void RadioWidget::on_PlayButton__released ()
	{
		const int idx = Ui_.ProviderBox_->currentIndex ();
		if (idx < 0)
			return;

		auto prov = Providers_.at (idx);
		auto station = prov->GetRadioStation (static_cast<Media::IRadioStationProvider::Type> (idx), Ui_.Param_->text ());
		Player_->SetRadioStation (station);
	}
}
}
