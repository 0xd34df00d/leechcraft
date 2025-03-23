/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "requesthandler.h"

#ifdef Q_OS_LINUX
#include <sys/sendfile.h>
#elif defined (Q_OS_FREEBSD)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#elif defined (Q_OS_MAC)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif

#include <errno.h>
#include <QList>
#include <QString>
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <util/util.h>
#include <util/sys/mimedetector.h>
#include <util/sll/util.h>
#include "connection.h"
#include "storagemanager.h"
#include "iconresolver.h"
#include "trmanager.h"

namespace LC
{
namespace HttHare
{
	RequestHandler::RequestHandler (const Connection_ptr& conn)
	: Conn_ (conn)
	{
		ResponseHeaders_.push_back ({ "Accept-Ranges", "bytes" });
	}

	void RequestHandler::operator() (QByteArray data)
	{
		data.replace ("\r", "");

		auto lines = data.split ('\n');
		for (auto& line : lines)
			line = line.trimmed ();
		lines.removeAll ({});

		if (lines.size () <= 0)
			return ErrorResponse (400, "Bad Request");

		const auto& req = lines.takeAt (0).split (' ');
		if (req.size () < 2)
			return ErrorResponse (400, "Bad Request");

		const auto& verb = req.at (0).toLower ();
		Url_ = QUrl::fromEncoded (req.at (1));

		for (const auto& line : lines)
		{
			const auto colonPos = line.indexOf (':');
			if (colonPos <= 0)
				return ErrorResponse (400, "Bad Request");
			Headers_ [line.left (colonPos)] = line.mid (colonPos + 1).trimmed ();
		}

#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO << "got request";
		qDebug () << req << Url_;
		for (auto i = Headers_.begin (); i != Headers_.end (); ++i)
			qDebug () << '\t' << i.key () << ": " << i.value ();
#endif

		if (verb == "head")
			HandleRequest (Verb::Head);
		else if (verb == "get")
			HandleRequest (Verb::Get);
		else
			return ErrorResponse (405, "Method Not Allowed",
					"Method " + verb + " not supported by this server.");
	}

	QString RequestHandler::Tr (const char *msg)
	{
		auto locales = Headers_ ["Accept-Language"].split (',');
		locales.removeAll ("*");
		for (auto& locale : locales)
		{
			const auto cpPos = locale.indexOf (';');
			if (cpPos >= 0)
				locale = locale.left (cpPos);
			locale = locale.trimmed ();
		}
		if (!locales.contains ("en"))
			locales << "en";

		auto mgr = Conn_->GetTrManager ();
		return mgr->Translate (locales, "LC::HttHare::RequestHandler", msg);
	}

	void RequestHandler::ErrorResponse (int code,
			const QByteArray& reason, const QByteArray& full)
	{
		ResponseLine_ = "HTTP/1.1 " + QByteArray::number (code) + " " + reason + "\r\n";

		ResponseBody_ = QString (R"delim(<html>
				<head><title>%1 %2</title></head>
				<body>
					<h1>%1 %2</h1>
					%3
				</body>
			</html>
			)delim")
				.arg (code)
				.arg (reason.data ())
				.arg (full.data ()).toUtf8 ();

		DefaultWrite (Verb::Get);
	}

	namespace
	{
		QString NormalizeClass (QString mime)
		{
			return mime.replace ('/', '_')
					.replace ('-', '_')
					.replace ('.', '_')
					.replace ('+', '_');
		}

		const auto IconSize = 16;
	}

	QByteArray RequestHandler::MakeDirResponse (const QFileInfo& fi, const QString& path, const QUrl& url)
	{
		const auto& entries = QDir { path }
				.entryInfoList (QDir::AllEntries | QDir::NoDot,
						QDir::Name | QDir::DirsFirst);

		struct MimeInfo
		{
			QString MimeType_;
		};
		QHash<QString, QByteArray> mimeCache;
		QList<MimeInfo> mimes;
		Util::MimeDetector detector;
		for (const auto& entry : entries)
		{
			const auto& type = detector (entry.filePath ());

			if (!mimeCache.contains (type))
			{
				QByteArray image;
				QMetaObject::invokeMethod (Conn_->GetIconResolver (),
						"resolveMime",
						Qt::BlockingQueuedConnection,
						Q_ARG (QString, type),
						Q_ARG (QByteArray&, image),
						Q_ARG (int, IconSize));
				mimeCache [type] = image;
			}

			mimes.append ({ type });
		}

		QString result;
		result += "<html><head><title>" + fi.fileName () + "</title><style>";
		for (auto pos = mimeCache.begin (); pos != mimeCache.end (); ++pos)
		{
			result += "." + NormalizeClass (pos.key ()) + " {";
			result += "background-image: url('" + pos.value () + "');";
			result += "background-repeat: no-repeat;";
			result += "padding-left: " + QString::number (IconSize + 4) + ";";
			result += "}";
		}
		result += "</style></head><body><h1>" + Tr ("Listing of %1").arg (url.toString ()) + "</h1>";
		result += "<table style='width: 100%'><tr>";
		result += QString ("<th style='width: 60%'>%1</th><th style='width: 20%'>%2</th><th style='width: 20%'>%3</th>")
					.arg (Tr ("Name"))
					.arg (Tr ("Size"))
					.arg (Tr ("Created"));

		for (int i = 0; i < entries.size (); ++i)
		{
			const auto& item = entries.at (i);

			auto link = QUrl::toPercentEncoding (item.fileName (), {}, "'");

			result += "<tr><td class=" + NormalizeClass (mimes.at (i).MimeType_) + "><a href='";
			result += link + "'>" + item.fileName () + "</a></td>";
			result += "<td>" + Util::MakePrettySize (item.size ()) + "</td>";
			result += "<td>" + QLocale {}.toString (item.birthTime (), QLocale::ShortFormat) + "</td></tr>";
		}

		result += "</table></body></html>";

		return result.toUtf8 ();
	}

	namespace
	{
		QList<QPair<qint64, qint64>> ParseRanges (QString str, qint64 fullSize)
		{
			QList<QPair<qint64, qint64>> result;

			const auto eqPos = str.indexOf ('=');
			if (eqPos >= 0)
				str = str.mid (eqPos + 1);

			const auto pcPos = str.indexOf (';');
			if (pcPos >= 0)
				str = str.left (pcPos);

			for (const auto& elem : QStringView { str }.split (',', Qt::SkipEmptyParts))
			{
				const auto dashPos = elem.indexOf ('-');

				if (dashPos < 0)
					continue;

				const auto& startStr = elem.left (dashPos);
				const auto& endStr = elem.mid (dashPos + 1);
				if (startStr.isEmpty ())
				{
					bool ok = false;
					const auto last = endStr.toLongLong (&ok);
					if (!ok)
						continue;

					if (last)
						result.append ({ fullSize - last - 1, fullSize - 1 });
				}
				else
				{
					bool ok = false;
					const auto last = endStr.isEmpty () ?
							(ok = true, fullSize - 1) :
							endStr.toLongLong (&ok);
					if (!ok)
						continue;

					ok = false;
					const auto first = startStr.toLongLong (&ok);
					if (!ok)
						continue;

					if (first <= last)
						result.append ({ first, last });
				}
			}

			for (const auto& range : result)
				if (!range.first && range.second == fullSize - 1)
					return {};

			return result;
		}
	}

	namespace
	{
#if !defined (Q_OS_LINUX) && !defined (Q_OS_FREEBSD) && !defined (Q_OS_MAC)
		QPair<int, qint64> DumbSendfile (const std::shared_ptr<QFile>& file,
				boost::asio::ip::tcp::socket& sock, int offset, int toTransfer)
		{
			if (!file->seek (offset))
				return { ESPIPE, 0 };

			const auto& ba = file->read (toTransfer);
			try
			{
				boost::asio::write (sock,
						boost::asio::buffer (ba.constData (), static_cast<size_t> (ba.size ())));
			}
			catch (const boost::system::system_error& e)
			{
				return { e.code ().value (), 0 };
			}

			return { 0, ba.size () };
		}
#endif

		struct Sendfiler
		{
			boost::asio::ip::tcp::socket& Sock_;
			std::shared_ptr<QFile> File_;
			off_t Offset_;

			QPair<qint64, qint64> CurrentRange_;
			QList<QPair<qint64, qint64>> TailRanges_;

			std::function<void (boost::system::error_code, ulong)> Handler_;

			void operator() (boost::system::error_code ec, ulong)
			{
				for (qint64 toTransfer = CurrentRange_.second - CurrentRange_.first + 1; toTransfer > 0; )
				{
					off_t offset = CurrentRange_.first;
#ifdef Q_OS_LINUX
					const auto rc = sendfile (Sock_.native_handle (),
							File_->handle (), &offset, toTransfer);
					const auto transferred = rc > 0 ? rc : 0;
					const auto errCode = rc > 0 ? 0 : errno;
#elif defined (Q_OS_FREEBSD)
					off_t transferred = toTransfer;
					const auto rc = sendfile (File_->handle (), Sock_.native_handle (),
							offset, toTransfer, nullptr, &transferred, 0);
					if (rc == -1)
						transferred = 0;
					const auto errCode = rc == -1 ? errno : 0;
#elif defined (Q_OS_MAC)
					auto transferred = toTransfer;
					auto errCode = sendfile (File_->handle (),
							Sock_.native_handle (),
							offset, &transferred,
							nullptr, 0);
#else
#warning "Using suboptimal file sending method"
					const auto& pair = DumbSendfile (File_, Sock_, offset, toTransfer);
					const auto errCode = pair.first;
					const auto transferred = pair.second;
#endif

					ec = boost::system::error_code (errCode,
							boost::asio::error::get_system_category ());

					if (!errCode)
					{
						CurrentRange_.first = offset;
						toTransfer -= transferred;
					}

					if (ec == boost::asio::error::interrupted)
						continue;

					if (ec == boost::asio::error::would_block ||
							ec == boost::asio::error::try_again)
					{
						Sock_.async_write_some (boost::asio::null_buffers {}, *this);
						return;
					}

					if (ec)
						break;

					if (!toTransfer && !TailRanges_.isEmpty ())
					{
						CurrentRange_ = TailRanges_.takeFirst ();
						Sock_.async_write_some (boost::asio::null_buffers {}, *this);
						return;
					}
				}

				Handler_ (ec, 0);
			}
		};
	}

	void RequestHandler::HandleRequest (Verb verb)
	{
		QString path;
		try
		{
			path = Conn_->GetStorageManager ().ResolvePath (Url_);
		}
		catch (const AccessDeniedException&)
		{
			ResponseLine_ = "HTTP/1.1 403 Forbidden\r\n";

			ResponseHeaders_.push_back ({ "Content-Type", "text/html; charset=utf-8" });
			ResponseBody_ = QString (R"delim(<html>
					<head><title>%2</title></head>
					<body>
						%1
					</body>
				</html>
				)delim")
					.arg (Tr ("Access forbidden. You could return "
								"to the <a href='/'>root</a> of this server.")
							.arg (path))
					.arg (QFileInfo { Url_.path () }.fileName ())
					.toUtf8 ();

			DefaultWrite (verb);

			return;
		}

		const QFileInfo fi { path };
		if (!fi.exists ())
		{
			ResponseLine_ = "HTTP/1.1 404 Not found\r\n";

			ResponseHeaders_.push_back ({ "Content-Type", "text/html; charset=utf-8" });
			ResponseBody_ = QString (R"delim(<html>
					<head><title>%1</title></head>
					<body>
						%2
					</body>
				</html>
				)delim")
					.arg (fi.fileName ())
					.arg (Tr ("%1 is not found on this server")
						.arg ("<em>" + path + "</em>"))
					.toUtf8 ();

			DefaultWrite (verb);
		}
		else if (fi.isDir ())
			WriteDir (path, fi, verb);
		else
			WriteFile (path, fi, verb);
	}

	void RequestHandler::WriteDir (const QString& path, const QFileInfo& fi, RequestHandler::Verb verb)
	{
		if (Url_.path ().endsWith ('/'))
		{
			ResponseLine_ = "HTTP/1.1 200 OK\r\n";

			ResponseHeaders_.push_back ({ "Content-Type", "text/html; charset=utf-8" });
			ResponseBody_ = MakeDirResponse (fi, path, Url_);

			DefaultWrite (verb);
		}
		else
		{
			ResponseLine_ = "HTTP/1.1 301 Moved Permanently\r\n";

			auto url = Url_;
			url.setPath (url.path () + '/');
			ResponseHeaders_.append ({ "Location", url.toString ().toUtf8 () });
			ResponseBody_ = MakeDirResponse (fi, path, url);

			DefaultWrite (verb);
		}
	}

	void RequestHandler::WriteFile (const QString& path, const QFileInfo& fi, RequestHandler::Verb verb)
	{
		auto ranges = ParseRanges (Headers_.value ("Range"), fi.size ());

		const auto& mime = Util::MimeDetector {} (path);
		ResponseHeaders_.append ({ "Content-Type", mime });

		if (ranges.isEmpty ())
		{
			ResponseLine_ = "HTTP/1.1 200 OK\r\n";
			ResponseHeaders_.append ({ "Content-Length", QByteArray::number (fi.size ()) });
		}
		else
		{
			ResponseLine_ = "HTTP/1.1 206 Partial content\r\n";

			qint64 totalSize = 0;
			for (const auto& range : ranges)
				totalSize += range.second - range.first + 1;

			ResponseHeaders_.append ({ "Content-Length", QByteArray::number (totalSize) });
		}

		auto c = Conn_;
		boost::asio::async_write (c->GetSocket (),
				ToBuffers (verb),
				c->GetStrand ().wrap ([c, path, verb, ranges] (boost::system::error_code ec, ulong) mutable -> void
					{
						if (ec)
							qWarning () << ec.message ().c_str ();

						auto& s = c->GetSocket ();

						auto shutdownGuard = Util::MakeScopeGuard ([&s, &ec]
								{ s.shutdown (boost::asio::socket_base::shutdown_both, ec); }).Shared ();

						if (verb != Verb::Get)
							return;

						auto file = std::make_shared<QFile> (path);
						if (!file->open (QIODevice::ReadOnly))
						{
							qWarning () << Q_FUNC_INFO
									<< "cannot open file"
									<< path
									<< file->errorString ();
							return;
						}

						if (ranges.isEmpty ())
							ranges.append ({ 0, file->size () - 1 });

						if (!s.native_non_blocking ())
							s.native_non_blocking (true, ec);

						const auto& headRange = ranges.takeFirst ();
						Sendfiler
						{
							s,
							std::move (file),
							0,
							headRange,
							ranges,
							[c, shutdownGuard] (boost::system::error_code, ulong) {}
						} (ec, 0);
					}));
	}

	void RequestHandler::DefaultWrite (Verb verb)
	{
		auto c = Conn_;
		boost::asio::async_write (c->GetSocket (),
				ToBuffers (verb),
				c->GetStrand ().wrap ([c] (const boost::system::error_code& ec, ulong)
					{
						if (ec)
							qWarning () << ec.message ().c_str ();

						boost::system::error_code iec;
						c->GetSocket ().shutdown (boost::asio::socket_base::shutdown_both, iec);
					}));
	}

	namespace
	{
		boost::asio::const_buffer BA2Buffer (const QByteArray& ba)
		{
			return { ba.constData (), static_cast<size_t> (ba.size ()) };
		}

		bool SupportsDeflate (const QStringList& ae)
		{
			for (const auto& val : ae)
				if (!val.trimmed ().compare ("deflate", Qt::CaseInsensitive))
					return true;

			return false;
		}
	}

	std::vector<boost::asio::const_buffer> RequestHandler::ToBuffers (Verb verb)
	{
		std::vector<boost::asio::const_buffer> result;

		const bool hasContentLength = std::any_of (ResponseHeaders_.begin (), ResponseHeaders_.end (),
				[] (const auto& pair) { return pair.first.toLower () == "content-length"; });

		const auto& splitAe = Headers_.value ("Accept-Encoding").split (',');
		if (verb == Verb::Get &&
				!ResponseBody_.isEmpty () &&
				SupportsDeflate (splitAe))
		{
			ResponseHeaders_.push_back ({ "Content-Encoding", "deflate" });
			ResponseBody_ = qCompress (ResponseBody_, 6);
			ResponseBody_.remove (0, 4);
		}

		if (!hasContentLength)
			ResponseHeaders_.append ({ "Content-Length", QByteArray::number (ResponseBody_.size ()) });

		CookedRH_.clear ();
		for (const auto& pair : ResponseHeaders_)
			CookedRH_ += pair.first + ": " + pair.second + "\r\n";
		CookedRH_ += "\r\n";

#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO;
		qDebug () << '\t' << ResponseLine_.left (ResponseLine_.size () - 2);
		for (const auto& pair : ResponseHeaders_)
			qDebug () << '\t' << (pair.first + ": " + pair.second);
#endif

		result.push_back (BA2Buffer (ResponseLine_));
		result.push_back (BA2Buffer (CookedRH_));

		if (verb == Verb::Get)
			result.push_back (BA2Buffer (ResponseBody_));

		return result;
	}
}
}
