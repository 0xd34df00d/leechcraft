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
#include <stacktrace>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QDateTime>
#include <QDir>

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
				stream << "[FTL] ";
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

	void PrintBacktrace (const std::shared_ptr<std::ostream>& ostr)
	{
		constexpr auto skip = 2;
		const auto trace = std::stacktrace::current (skip);
		for (const auto& e : trace)
			*ostr << '\t' << e << '\n';
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

	QByteArray DetectFile (const QMessageLogContext& ctx)
	{
		if (!ctx.file)
			return "<unk>";

		QByteArray file { ctx.file };
		const auto pos = file.lastIndexOf ('/');
		return pos >= 0 ? file.mid (pos + 1) : file;
	};

	QByteArray DetectLine (const QMessageLogContext& ctx)
	{
		return ctx.line > 0 ? ':' + QByteArray::number (ctx.line) : QByteArray {};
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
		const auto& module = Colorize (flags & DebugWriteFlag::DWFNoFileLog, DetectModule (ctx))
				+ ':' + Colorize (flags & DebugWriteFlag::DWFNoFileLog, DetectFile (ctx))
				+ ':' + DetectLine (ctx);

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
