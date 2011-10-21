/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include "separateplayerwidget.h"
#include "core.h"

namespace LeechCraft
{
namespace Laure
{
	const int pos_slider_max = 10000;
		
	Player::Player (QWidget *parent)
	: QFrame (parent)
	, Poller_ (new QTimer (this))
	, Core_ (new Core (this))
	{
		Core_->setWindow (winId ());
		
		connect (Poller_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (timeout ()));
		connect (Poller_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleTimeout ()));
		Poller_->start (300);
	}
	
	Core* Player::GetCore ()
	{
		return Core_;
	}
	
	QTime Player::Time ()
	{
		return IntToQTime (Core_->Time ());
	}
	
	QTime Player::Length ()
	{
		return IntToQTime (Core_->Length ());
	}
	
	int Player::Position () const
	{
		if (!Core_->IsPlaying ())
			return -1;

		return Core_->MediaPosition () * static_cast<float> (pos_slider_max);
	}


	void Player::setPosition (int pos)
	{
		Core_->setPosition (static_cast<float> (pos) / pos_slider_max);
	}
	
	void Player::handleTimeout ()
	{
		int time = Core_->Time ();
		int length = Core_->Length ();
		if (length - time < 200 && Core_->IsPlaying ())
			Core_->next ();
	}
	
	QTime Player::IntToQTime (int val)
	{
		QTime time = QTime (0, 0);
		return val < 0 ? time : time.addMSecs (val);
	}
}
}

