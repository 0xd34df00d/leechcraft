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

#include "iodevicesink.h"

namespace LeechCraft
{
namespace Azoth
{
	IODeviceSink::IODeviceSink (QIODevice *dev)
	: Device_ (dev)
	{
	}

	void IODeviceSink::eos ()
	{
	}

	QGst::FlowReturn IODeviceSink::newBuffer ()
	{
		QGst::BufferPtr buf = pullBuffer ();

		QByteArray arr;
		arr.resize (buf->size ());
		for (int i = 0, size = buf->size (); i < size; ++i)
			arr [i] = buf->data () [i];

		Device_->write (arr);

		return QGst::FlowOk;
	}
}
}
