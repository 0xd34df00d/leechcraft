/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#ifndef UTIL_VERSIONACTIONMAPPER_H
#define UTIL_VERSIONACTIONMAPPER_H
#include <functional>
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
			typedef std::function<bool (QDataStream&)> Functor_t;
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
