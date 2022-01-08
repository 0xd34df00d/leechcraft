/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "debugmessagehandler.h"
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <optional>

#if defined (_GNU_SOURCE) || defined (Q_OS_OSX)
#include <execinfo.h>
#include <cxxabi.h>
#include <sys/types.h>
#endif

#include <unistd.h>

#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QHash>
#include <util/sll/monad.h>

QMutex G_DbgMutex;
uint Counter = 0;

namespace
{
	QString GetFilename (QtMsgType type)
	{
		switch (type)
		{
		case QtDebugMsg:
			return "debug.log";
		case QtInfoMsg:
			return "info.log";
		case QtWarningMsg:
			return "warning.log";
		case QtCriticalMsg:
			return "critical.log";
		case QtFatalMsg:
			return "fatal.log";
		}

		return "unknown.log";
	}

	bool SupportsColors ()
	{
		static const auto supportsColors = isatty (fileno (stdout));
		return supportsColors;
	}

	std::string GetColorCode (QtMsgType type)
	{
		if (!SupportsColors ())
			return {};

		switch (type)
		{
		case QtDebugMsg:
		case QtInfoMsg:
			return "\x1b[32m";
		case QtWarningMsg:
			return "\x1b[33m";
		case QtCriticalMsg:
			return "\x1b[31m";
		case QtFatalMsg:
			return "\x1b[35m";
		}

		return {};
	}

	std::shared_ptr<std::ostream> GetOstream (QtMsgType type, DebugHandler::DebugWriteFlags flags)
	{
		if (flags & DebugHandler::DWFNoFileLog)
		{
			auto& stream = type == QtDebugMsg ? std::cout : std::cerr;

			stream << GetColorCode (type);
			switch (type)
			{
			case QtDebugMsg:
				stream << "[DBG] ";
				break;
			case QtInfoMsg:
				stream << "[INF] ";
				break;
			case QtWarningMsg:
				stream << "[WRN] ";
				break;
			case QtCriticalMsg:
				stream << "[CRT] ";
				break;
			case QtFatalMsg:
				stream << "[FTL] (yay, really `faster than light`!) ";
				break;
			}
			if (SupportsColors ())
				stream << "\x1b[0m";

			return { &stream, [] (std::ostream*) {} };
		}

		const QString name = QDir::homePath () + "/.leechcraft/" + GetFilename (type);

		auto ostr = std::make_shared<std::ofstream> ();
		ostr->open (QDir::toNativeSeparators (name).toStdString (), std::ios::app);
		return ostr;
	}

	struct AddrInfo
	{
		std::string ObjectPath_;
		std::string SourcePath_;
		std::string Symbol_;
	};

#if defined (_GNU_SOURCE)
	using StrRange_t = std::pair<const char*, const char*>;

	std::optional<StrRange_t> FindStrRange (const char *str, char open, char close)
	{
		const auto strEnd = str + std::strlen (str);
		const auto parensCount = std::count_if (str, strEnd,
				[=] (char c) { return c == open || c == close; });
		if (parensCount != 2)
			return {};

		const auto openParen = std::find (str, strEnd, open);
		const auto closeParen = std::find (openParen, strEnd, close);
		if (openParen == strEnd || closeParen == strEnd)
			return {};

		return { { openParen + 1, closeParen } };
	}

	std::optional<AddrInfo> QueryAddr2Line (const std::string& execName,
			const std::string& addr)
	{
		QProcess proc;
		proc.start ("addr2line",
				{ "-Cfe", QString::fromStdString (execName), QString::fromStdString (addr) });
		if (!proc.waitForFinished (500))
			return {};

		if (proc.exitStatus () != QProcess::NormalExit ||
			proc.exitCode ())
			return {};

		const auto& out = proc.readAllStandardOutput ().trimmed ().split ('\n');
		if (out.size () != 2)
			return {};

		return { { execName, out [1].constData (), out [0].constData () } };
	}

	std::optional<AddrInfo> QueryAddr2LineExecutable (const char *str,
			const std::string& execName)
	{
		using LC::Util::operator>>;

		return FindStrRange (str, '[', ']') >>
				[&] (const auto& bracketRange)
				{
					return QueryAddr2Line (execName,
							{ bracketRange.first, bracketRange.second });
				};
	}

	std::optional<AddrInfo> QueryAddr2LineLibrary (const std::string& execName,
			const std::string& addr)
	{
		return QueryAddr2Line (execName, addr);
	}

	std::optional<std::string> GetDemangled (const char *str)
	{
		int status = -1;
		const auto demangled = abi::__cxa_demangle (str, nullptr, 0, &status);

		if (!demangled)
			return {};

		const std::string demangledStr { demangled };
		free (demangled);
		return demangledStr;
	}

	class AddrInfoGetter
	{
		const QHash<QString, size_t> LibAddrsCache_;
	public:
		AddrInfoGetter ()
		: LibAddrsCache_ { ParseLibAddrs () }
		{
		}

		std::optional<AddrInfo> operator() (const char *str) const
		{
			using LC::Util::operator>>;

			return FindStrRange (str, '(', ')') >>
					[this, str] (const StrRange_t& pair)
					{
						const std::string binaryName { str, pair.first - 1 };

						const auto plusPos = std::find (pair.first, pair.second, '+');

						if (plusPos == pair.second)
							return QueryAddr2LineExecutable (str, binaryName);

						if (plusPos == pair.first)
							return QueryAddr2LineLibrary (binaryName, { plusPos, pair.second });

						if (const auto relative = QueryRelative (binaryName, str))
							return relative;

						return GetDemangled (pair.first) >>
								[&] (const std::string& value)
								{
									return std::optional<AddrInfo> { { binaryName, {}, value } };
								};
					};
		}
	private:
		std::optional<AddrInfo> QueryRelative (const std::string& binaryName, const char *str) const
		{
			const auto& symLinked = QFile::symLinkTarget (QString::fromStdString (binaryName));
			const auto libAddrPos = LibAddrsCache_.find (symLinked);
			if (libAddrPos == LibAddrsCache_.end ())
				return {};

			using LC::Util::operator>>;

			return FindStrRange (str, '[', ']') >>
					[&] (const auto& bracketRange) -> std::optional<AddrInfo>
					{
						try
						{
							const auto addr = std::stoull (std::string {
										bracketRange.first,
										bracketRange.second
									},
									nullptr, 16);
							std::stringstream ss;
							ss << "0x" << std::hex << addr - libAddrPos.value ();
							return QueryAddr2LineLibrary (binaryName, ss.str ());
						}
						catch (const std::invalid_argument&)
						{
						}

						return {};
					};
		}

		static QHash<QString, size_t> ParseLibAddrs ()
		{
			QFile file { QString { "/proc/%1/maps" }.arg (getpid ()) };
			if (!file.open (QIODevice::ReadOnly))
				return {};

			QHash<QString, size_t> result;
			for (const auto& line : file.readAll ().split ('\n'))
			{
				if (!line.contains ("r-xp"))
					continue;

				const auto lastSpaceIdx = line.lastIndexOf (' ');
				const auto dashIdx = line.indexOf ('-');
				if (lastSpaceIdx == -1)
					continue;

				const QString libName { line.begin () + lastSpaceIdx + 1 };

				try
				{
					const auto baseAddr = std::stoull (std::string {
								line.begin (),
								line.begin () + dashIdx
							},
							nullptr, 16);
					result.insert (libName, baseAddr);
				}
				catch (const std::invalid_argument&)
				{
				}
			}
			return result;
		}
	};
#elif defined (Q_OS_OSX)
	class AddrInfoGetter
	{
	public:
		std::optional<AddrInfo> operator() (const char*)
		{
			return {};
		}
	};
#endif

	void PrintBacktrace (const std::shared_ptr<std::ostream>& ostr)
	{
#if defined (_GNU_SOURCE) || defined (Q_OS_OSX)
		const int maxSize = 100;
		void *callstack [maxSize];
		size_t size = backtrace (callstack, maxSize);
		char **strings = backtrace_symbols (callstack, size);

		*ostr << "Backtrace of " << size << " frames:" << std::endl;

		AddrInfoGetter getter;

		for (size_t i = 0; i < size; ++i)
		{
			*ostr << i << "\t";

			if (const auto info = getter (strings [i]))
				*ostr << info->ObjectPath_
						<< ": "
						<< info->Symbol_
						<< " ["
						<< info->SourcePath_
						<< "]"
						<< std::endl;
			else
				*ostr << strings [i] << std::endl;
		}

		std::free (strings);
#endif
	}

	QByteArray DetectModule (const QMessageLogContext& ctx)
	{
		if (!ctx.file)
			return "<unk>";

		const auto& file = QByteArray::fromRawData (ctx.file, std::strlen (ctx.file));
		if (file.isEmpty ())
			return "<unk>";

		static const QByteArray pluginsMarker { "src/plugins/" };
		const auto pluginsPos = file.indexOf (pluginsMarker);
		if (pluginsPos != -1)
		{
			const auto pluginNameStart = pluginsPos + pluginsMarker.size ();
			const auto nextSlash = file.indexOf ('/', pluginNameStart + 1);
			return nextSlash >= 0 ?
					file.mid (pluginNameStart, nextSlash - pluginNameStart) :
					file.mid (pluginNameStart);
		}

		if (file.contains ("src/core/"))
			return "core";

		if (file.contains ("src/util/"))
			return "util";

		if (file.contains ("src/xmlsettingsdialog/"))
			return "xsd";

		if (file.endsWith (".qml"))
		{
			const auto lastSlash = file.lastIndexOf ('/');
			const auto prelastSlash = file.lastIndexOf ('/', lastSlash - 1);
			if (lastSlash >= 0 && prelastSlash >= 0)
				return file.mid (prelastSlash + 1, lastSlash - prelastSlash - 1);
		}

		return file;
	}

	auto MakeColorTable ()
	{
		static const auto colorfgbg = QString::fromLatin1 (qgetenv ("COLORFGBG")).section (';', 1);

		bool dark = false;
		bool light = false;

		if (!colorfgbg.isEmpty ())
		{
			bool ok;
			if (const auto num = colorfgbg.toInt (&ok); ok)
			{
				dark = num >= 0 && num <= 8 && num != 7;
				light = !dark;
			}
		}

		QList<QByteArray> result;
		for (int i = 30; i <= 37; ++i)
		{
			if ((i == 30 && dark) || (i == 37 && light))
				continue;

			auto str = QByteArray::number (i);
			result << str
					<< "1;" + str;
		}

		return result;
	}

	QByteArray Colorize (bool shouldColorize, const QByteArray& str)
	{
		if (!shouldColorize)
			return str;

		static const auto table = MakeColorTable ();

		const auto hash = qHash (str) % table.size ();
		return "\x1b[" + table [hash] + "m" + str + "\x1b[0m";
	}
}

namespace DebugHandler
{
	void Write (QtMsgType type, const QMessageLogContext& ctx, const char *message, DebugWriteFlags flags)
	{
#if !defined (Q_OS_WIN32)
		if (!strcmp (message, "QPixmap::handle(): Pixmap is not an X11 class pixmap") ||
				strstr (message, ": Painter not active"))
			return;
#endif
#if defined (Q_OS_WIN32)
		if (!strcmp (message, "QObject::startTimer: QTimer can only be used with threads started with QThread"))
			return;
#endif

		const auto& now = QDateTime::currentDateTime ().toString ("dd.MM.yyyy HH:mm:ss.zzz").toStdString ();
		const auto& thread = QString { "0x%1" }
				.arg (reinterpret_cast<uintptr_t> (QThread::currentThread ()), 16, 16, QChar { '0' })
				.toStdString ();
		const auto& module = Colorize (flags & DebugWriteFlag::DWFNoFileLog, DetectModule (ctx));

		QMutexLocker locker { &G_DbgMutex };

		const auto& ostr = GetOstream (type, flags);
		*ostr << "["
				<< now
				<< "] ["
				<< std::setfill ('0')
				<< std::setw (3)
				<< Counter++
				<< "] ["
				<< thread
				<< "] ["
				<< module.constData ()
				<< "] "
				<< message
				<< std::endl;

		if (type != QtDebugMsg && (flags & DWFBacktrace))
			PrintBacktrace (ostr);
	}
}
