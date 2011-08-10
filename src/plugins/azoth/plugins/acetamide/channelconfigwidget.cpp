/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "channelconfigwidget.h"
#include "channelclentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelConfigWidget::ChannelConfigWidget (ChannelCLEntry *clentry, QWidget *parent) 
	: QWidget (parent)
	, Channel_ (clentry)
	, ChannelMode_ (clentry->GetChannelModes ())
	{
		Ui_.setupUi (this);

		SetModesUi ();
	}

	void ChannelConfigWidget::SetModesUi ()
	{
		Ui_.OpTopic_->setChecked (ChannelMode_.OnlyOpChangeTopicMode_);
		Ui_.BlockOutMessage_->setChecked (ChannelMode_.BlockOutsideMessageMode_);
		Ui_.SecretChannel_->setChecked (ChannelMode_.SecretMode_);
		Ui_.PrivateChannel_->setChecked (ChannelMode_.PrivateMode_);
		Ui_.InvitesOnly_->setChecked (ChannelMode_.InviteMode_);
		Ui_.ModerateChannel_->setChecked (ChannelMode_.ModerateMode_);
		Ui_.ReOp_->setChecked (ChannelMode_.ReOpMode_);
		Ui_.UserLimit_->setChecked (ChannelMode_.UserLimit_.first);
		Ui_.Limit_->setValue (ChannelMode_.UserLimit_.second);
		Ui_.Password_->setChecked (ChannelMode_.ChannelKey_.first);
		Ui_.Key_->setText (ChannelMode_.ChannelKey_.second);
	}

	void ChannelConfigWidget::accept ()
	{
	}

}
}
}