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

#ifndef PLUGININTERFACE_VERSIONACTIONMAPPER_H
#define PLUGININTERFACE_VERSIONACTIONMAPPER_H
#include <boost/function.hpp>
#include <QDataStream>
#include <QMap>

namespace LeechCraft
{
	namespace Util
	{
		template<typename ActionType, typename VerType = quint8>
		class VersionActionMapper
		{
		public:
			typedef boost::function<bool (QDataStream&)> Functor_t;
		private:
			typedef QMap<ActionType, Functor_t> Action2Functor_t;
			typedef QMap<VerType, Action2Functor_t> Version2Functors_t;
			Version2Functors_t Functors_;
		public:
			struct Simple
			{
				Functor_t F_;

				Simple (Functor_t f)
				: F_ (f)
				{
				}

				bool operator() (QDataStream& ds)
				{
					F_ (ds);
					return true;
				}
			};

			VersionActionMapper ()
			{
			}

			void AddFunctor (VerType version, ActionType action,
					Functor_t functor)
			{
				Functors_ [version] [action] = functor;
			}

			bool Process (const QByteArray& ba)
			{
				QDataStream in (ba);
				return Process (in);
			}

			bool Process (QDataStream& in)
			{
				quint8 version = 0;
				in >> version;
				if (!Functors_.contains (version))
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown version"
							<< version;
					return false;
				}

				quint16 action = 0;
				in >> action;
				if (in.status () != QDataStream::Ok)
				{
					qWarning () << Q_FUNC_INFO
							<< "bad status"
							<< in.status ()
							<< "for version"
							<< version;
					return false;
				}

				ActionType act = static_cast<ActionType> (action);
				if (!Functors_ [version].contains (act))
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown action"
							<< act
							<< "for version"
							<< version;
					return false;
				}

				return Functors_ [version] [act] (in);
			}
		};
	}
}

#endif
