/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QFile>
#include <interfaces/azoth/ihaveconsole.h>

class QVariant;
class QUrl;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class Logger : public QObject
	{
		Q_OBJECT

		const QString Filename_;
		bool FileEnabled_ = true;
	public:
		class LogProxy
		{
			friend class Logger;

			Logger& L_;
			const IHaveConsole::PacketDirection Dir_;

			bool IsFirst_ = true;
			std::unique_ptr<QFile> File_;

			QByteArray CurrentString_;

			LogProxy (Logger&, IHaveConsole::PacketDirection);
		public:
			LogProxy (const LogProxy&) = delete;
			LogProxy (LogProxy&&) = default;

			~LogProxy ();

			template<typename T>
			LogProxy operator<< (const T& t)
			{
				if (!IsFirst_)
					Write (" ");
				Write (t);
				IsFirst_ = false;
				return std::move (*this);
			}
		private:
			void Write (const char*);
			void Write (const QString&);
			void Write (qint64);
			void Write (const QUrl&);
			void Write (const QVariant&);

			void WriteImpl (const QByteArray&);

			template<typename T>
			void Write (const QList<T>& list)
			{
				Write ("[ ");
				bool isFirst = true;
				for (const auto& value : list)
				{
					if (!isFirst)
						Write ("; ");
					isFirst = false;
					Write (value);
				}
				Write (" ]");
			}
		};

		Logger (const QString& id, QObject* = 0);

		void SetFileEnabled (bool);

		LogProxy operator() (IHaveConsole::PacketDirection dir)
		{
			return LogProxy { *this, dir };
		}

		template<typename T>
		LogProxy operator<< (const T& t)
		{
			return LogProxy { *this, IHaveConsole::PacketDirection::In } << t;
		}
	signals:
		void gotConsolePacket (const QByteArray&, IHaveConsole::PacketDirection, const QString&);
	};
}
}
}
