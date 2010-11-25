/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGININTERFACE_IDPOOL_H
#define PLUGININTERFACE_IDPOOL_H
#include <QByteArray>
#include <QSet>
#include <QDataStream>
#include <QtDebug>
#include "plugininterface/guarded.h"

namespace LeechCraft
{
	namespace Util
	{
		template<typename T>
		class IDPool
		{
			T CurrentID_;
		public:
			IDPool (T id=T ())
			: CurrentID_ (id)
			{
			}

			virtual ~IDPool ()
			{
			}

			T GetID ()
			{
				return CurrentID_++;
			}

			void SetID(T id)
			{
				CurrentID_ = id;
			}

			void FreeID (T id)
			{
				if (id == CurrentID_)
					--CurrentID_;
			}

			QByteArray SaveState () const
			{
				QByteArray result;
				{
					QDataStream ostr (&result, QIODevice::WriteOnly);
					quint8 ver = 1;
					ostr << ver;
					ostr << CurrentID_;
				}
				return result;
			}

			void LoadState (const QByteArray& state)
			{
				if (state.isEmpty ())
					return;

				QDataStream istr (state);
				quint8 ver;
				istr >> ver;
				if (ver == 1)
					istr >> CurrentID_;
				else
					qWarning () << Q_FUNC_INFO
							<< "unknown version"
							<< ver
							<< ", not restoring state.";
			}
		};
	};
};

#endif
