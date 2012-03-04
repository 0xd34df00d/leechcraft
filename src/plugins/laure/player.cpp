/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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

#include "player.h"
#include <QDebug>
#include <QSlider>
#include <QTimer>
#include <QPushButton>
#include <QTime>
#include "vlcwrapper.h"

namespace LeechCraft
{
namespace Laure
{
	const int pos_slider_max = 10000;
		
	Player::Player (QWidget *parent)
	: QFrame (parent)
	, Poller_ (new QTimer (this))
	{
		connect (Poller_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (timeout ()));
		Poller_->start (300);
	}
	
	void Player::SetVLCWrapper (std::shared_ptr<VLCWrapper> core)
	{
		VLCWrapper_ = core;
		VLCWrapper_->setWindow (winId ());
	}
	
	namespace
	{
		QTime IntToQTime (int val)
		{
			return val < 0 ? QTime () : QTime (0, 0).addMSecs (val);
		}
	}
	
	QTime Player::GetTime () const
	{
		if (!VLCWrapper_)
			return QTime ();
		
		return IntToQTime (VLCWrapper_->GetTime ());
	}
	
	QTime Player::GetLength () const
	{
		if (!VLCWrapper_)
			return QTime ();
		
		return IntToQTime (VLCWrapper_->GetLength ());
	}
	
	int Player::GetPosition () const
	{
		if (!(VLCWrapper_ && VLCWrapper_->IsPlaying ()))
			return -1;

		return VLCWrapper_->GetMediaPosition () * pos_slider_max;
	}

	void Player::setPosition (int pos)
	{
		if (!VLCWrapper_)
			return;
		
		VLCWrapper_->setPosition (static_cast<float> (pos) / pos_slider_max);
	}
}
}

