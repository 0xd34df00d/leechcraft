/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "iodevicesrc.h"

namespace LeechCraft
{
namespace Azoth
{
	IODeviceSrc::IODeviceSrc (QIODevice *device)
	: Device_ (device)
	, IsEnoughData_ (false)
	{
		setStreamType (QGst::AppStreamTypeStream);
		enableBlock (false);

		connect (device,
				SIGNAL (readyRead ()),
				this,
				SLOT (handleReadyRead ()));
	}

	void IODeviceSrc::needData (uint length)
	{
		IsEnoughData_ = false;

		const QByteArray& data = Device_->read (length);
		QGst::BufferPtr buf = QGst::Buffer::create (data.size ());
		for (int i = 0, size = data.size (); i < size; ++i)
			buf->data () [i] = data.at (i);
		pushBuffer (buf);
	}

	void IODeviceSrc::enoughData ()
	{
		IsEnoughData_ = true;
	}

	void IODeviceSrc::handleReadyRead ()
	{
		if (IsEnoughData_)
			return;

		needData (std::min (Device_->bytesAvailable (), static_cast<qint64> (16384)));
	}
}
}
