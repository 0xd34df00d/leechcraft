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

#ifndef PLUGINS_AZOTH_IODEVICESRC_H
#define PLUGINS_AZOTH_IODEVICESRC_H
#include <QGst/Global>
#define QTGSTREAMERUTILS_EXPORT Q_DECL_IMPORT
#include <QGst/Utils/ApplicationSource>
#undef QTGSTREAMERUTILS_EXPORT

namespace LeechCraft
{
namespace Azoth
{
	class IODeviceSrc : public QObject
					  , public QGst::Utils::ApplicationSource
	{
		Q_OBJECT

		QIODevice *Device_;
		bool IsEnoughData_;
	public:
		IODeviceSrc (QIODevice*);
	protected:
		void needData (uint);
		void enoughData ();
	private slots:
		void handleReadyRead ();
	};
}
}

#endif
