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

#include "accountthreadworker.h"
#include <QMutexLocker>
#include <QUrl>
#include <QFile>
#include <QTimer>
#include <QtDebug>
#include <vmime/security/defaultAuthenticator.hpp>
#include <vmime/security/cert/defaultCertificateVerifier.hpp>
#include <vmime/security/cert/X509Certificate.hpp>
#include <vmime/net/transport.hpp>
#include <vmime/net/store.hpp>
#include <vmime/net/message.hpp>
#include <vmime/utility/datetimeUtils.hpp>
#include <vmime/dateTime.hpp>
#include <vmime/messageParser.hpp>
#include <vmime/messageBuilder.hpp>
#include <vmime/htmlTextPart.hpp>
#include <vmime/stringContentHandler.hpp>
#include <util/util.h>
#include "message.h"
#include "account.h"
#include "core.h"
#include "progresslistener.h"
#include "storage.h"
#include "vmimeconversions.h"
#include "outputiodevadapter.h"

namespace LeechCraft
{
namespace Snails
{
	namespace
	{
		class VMimeAuth : public vmime::security::defaultAuthenticator
		{
			Account::Direction Dir_;
			Account *Acc_;
		public:
			VMimeAuth (Account::Direction, Account*);

			const vmime::string getUsername () const;
			const vmime::string getPassword () const;
		private:
			QByteArray GetID () const
			{
				QByteArray id = "org.LeechCraft.Snails.PassForAccount/" + Acc_->GetID ();
				id += Dir_ == Account::Direction::Out ? "/Out" : "/In";
				return id;
			}
		};

		VMimeAuth::VMimeAuth (Account::Direction dir, Account *acc)
		: Dir_ (dir)
		, Acc_ (acc)
		{
		}

		const vmime::string VMimeAuth::getUsername () const
		{
			switch (Dir_)
			{
			case Account::Direction::Out:
				return Acc_->GetOutUsername ().toUtf8 ().constData ();
			default:
				return Acc_->GetInUsername ().toUtf8 ().constData ();
			}
		}

		const vmime::string VMimeAuth::getPassword () const
		{
			QString pass;

			QMetaObject::invokeMethod (Acc_,
					"getPassword",
					Qt::BlockingQueuedConnection,
					Q_ARG (QString*, &pass));

			return pass.toUtf8 ().constData ();
		}

		class CertVerifier : public vmime::security::cert::defaultCertificateVerifier
		{
			static std::vector<vmime::ref<vmime::security::cert::X509Certificate>> TrustedCerts_;
		public:
			void verify (vmime::ref<vmime::security::cert::certificateChain> chain)
			{
				try
				{
					setX509TrustedCerts (TrustedCerts_);

					defaultCertificateVerifier::verify (chain);
				}
				catch (vmime::exceptions::certificate_verification_exception&)
				{
					auto cert = chain->getAt (0);

					if (cert->getType() == "X.509")
						TrustedCerts_.push_back (cert.dynamicCast
								<vmime::security::cert::X509Certificate>());
				}
			}
		};

		std::vector<vmime::ref<vmime::security::cert::X509Certificate>> CertVerifier::TrustedCerts_;
	}

	class TimerGuard
	{
		QTimer *T_;
		int Timeout_;
	public:
		TimerGuard (QTimer *t, int timeout = 30000)
		: T_ (t)
		, Timeout_ (timeout)
		{
			T_->stop ();
		}

		~TimerGuard ()
		{
			T_->start (Timeout_);
		}
	};

	AccountThreadWorker::AccountThreadWorker (Account *parent)
	: A_ (parent)
	, DisconnectTimer_ (new QTimer (this))
	{
		Session_ = vmime::create<vmime::net::session> ();

		connect (DisconnectTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (timeoutDisconnect ()));
	}

	vmime::utility::ref<vmime::net::store> AccountThreadWorker::MakeStore ()
	{
		if (CachedStore_)
			return CachedStore_;

		QString url;

		QMetaObject::invokeMethod (A_,
				"buildInURL",
				Qt::BlockingQueuedConnection,
				Q_ARG (QString*, &url));

		auto st = Session_->getStore (vmime::utility::url (url.toUtf8 ().constData ()));
		st->setCertificateVerifier (vmime::create<CertVerifier> ());

		st->setProperty ("connection.tls", A_->UseTLS_);
		st->setProperty ("connection.tls.required", A_->TLSRequired_);
		st->setProperty ("options.sasl", A_->UseSASL_);
		st->setProperty ("options.sasl.fallback", A_->SASLRequired_);

		CachedStore_ = st;

		st->connect ();

		return st;
	}

	vmime::utility::ref<vmime::net::transport> AccountThreadWorker::MakeTransport ()
	{
		QString url;

		QMetaObject::invokeMethod (A_,
				"buildOutURL",
				Qt::BlockingQueuedConnection,
				Q_ARG (QString*, &url));

		QString username;
		QString password;
		bool setAuth = false;
		if (A_->SMTPNeedsAuth_ &&
				A_->OutType_ == Account::OutType::SMTP)
		{
			setAuth = true;

			QUrl parsed = QUrl::fromEncoded (url.toUtf8 ());
			username = parsed.userName ();
			password = parsed.password ();

			parsed.setUserName (QString ());
			parsed.setPassword (QString ());
			url = QString::fromUtf8 (parsed.toEncoded ());
		}

		auto trp = Session_->getTransport (vmime::utility::url (url.toUtf8 ().constData ()));

		if (setAuth)
		{
			trp->setProperty ("options.need-authentication", true);
			trp->setProperty ("auth.username", username.toUtf8 ().constData ());
			trp->setProperty ("auth.password", password.toUtf8 ().constData ());
		}
		trp->setCertificateVerifier (vmime::create<CertVerifier> ());
		trp->setProperty ("connection.tls", A_->UseTLS_);
		trp->setProperty ("connection.tls.required", A_->TLSRequired_);
		trp->setProperty ("options.sasl", true);
		trp->setProperty ("options.sasl.fallback", A_->SASLRequired_);

		return trp;
	}

	Message_ptr AccountThreadWorker::FromHeaders (const vmime::ref<vmime::net::message>& message) const
	{
		auto utf8cs = vmime::charset ("utf-8");

		Message_ptr msg (new Message);
		msg->SetID (message->getUniqueId ().c_str ());
		msg->SetSize (message->getSize ());

		if (message->getFlags () & vmime::net::message::FLAG_SEEN)
			msg->SetRead (true);

		auto header = message->getHeader ();

		try
		{
			auto mbox = header->From ()->getValue ().dynamicCast<const vmime::mailbox> ();
			auto pair = Mailbox2Strings (mbox);
			msg->SetFromEmail (pair.second);
			msg->SetFrom (pair.first);
		}
		catch (const vmime::exceptions::no_such_field& nsf)
		{
			qWarning () << "no 'from' data";
		}

		try
		{
			auto val = header->To ()->getValue ();
			auto alist = val.dynamicCast<const vmime::mailboxList> ();
			if (alist)
			{
				const auto& vec = alist->getMailboxList ();

				QList<QPair<QString, QString>> to;
				std::transform (vec.begin (), vec.end (), std::back_inserter (to),
						[] (decltype (vec.front ()) add) { return Mailbox2Strings (add); });
				msg->SetTo (to);
			}
			else
				qWarning () << "no 'to' data: cannot cast to mailbox list";
		}
		catch (const vmime::exceptions::no_such_field& nsf)
		{
			qWarning () << "no 'to' data" << nsf.what ();
		}

		try
		{
			auto origDate = header->Date ()->getValue ().dynamicCast<const vmime::datetime> ();
			auto date = vmime::utility::datetimeUtils::toUniversalTime (*origDate);
			QDate qdate (date.getYear (), date.getMonth (), date.getDay ());
			QTime time (date.getHour (), date.getMinute (), date.getSecond ());
			msg->SetDate (QDateTime (qdate, time, Qt::UTC));
		}
		catch (const vmime::exceptions::no_such_field&)
		{
		}

		try
		{
			auto str = header->Subject ()->getValue ()
					.dynamicCast<const vmime::text> ()->getConvertedText (utf8cs);
			msg->SetSubject (QString::fromUtf8 (str.c_str ()));
		}
		catch (const vmime::exceptions::no_such_field&)
		{
		}

		return msg;
	}

	namespace
	{
		vmime::ref<vmime::message> FromNetMessage (vmime::ref<vmime::net::message> msg)
		{
			return msg->getParsedMessage ();
		}
	}

	void AccountThreadWorker::FetchMessagesPOP3 (Account::FetchFlags fetchFlags)
	{
		auto store = MakeStore ();
		auto folder = store->getDefaultFolder ();
		folder->open (vmime::net::folder::MODE_READ_WRITE);

		auto messages = folder->getMessages ();
		if (!messages.size ())
			return;

		qDebug () << "know about" << messages.size () << "messages";
		int desiredFlags = vmime::net::folder::FETCH_FLAGS |
					vmime::net::folder::FETCH_SIZE |
					vmime::net::folder::FETCH_UID |
					vmime::net::folder::FETCH_ENVELOPE;
		desiredFlags &= folder->getFetchCapabilities ();

		qDebug () << "folder supports" << folder->getFetchCapabilities ()
				<< "so we gonna fetch" << desiredFlags;

		try
		{
			auto context = tr ("Fetching headers for %1")
					.arg (A_->GetName ());
			folder->fetchMessages (messages,
					desiredFlags, MkPgListener (context));
		}
		catch (const vmime::exceptions::operation_not_supported& ons)
		{
			qWarning () << Q_FUNC_INFO
					<< "fetch operation not supported:"
					<< ons.what ();
			return;
		}

		if (fetchFlags & Account::FetchNew)
		{
			if (folder->getFetchCapabilities () & vmime::net::folder::FETCH_FLAGS)
			{
				auto pos = std::remove_if (messages.begin (), messages.end (),
						[] (decltype (messages.front ()) msg) { return msg->getFlags () & vmime::net::message::FLAG_SEEN; });
				messages.erase (pos, messages.end ());
				qDebug () << "fetching only new msgs:" << messages.size ();
			}
			else
			{
				qDebug () << "folder hasn't advertised support for flags :(";
			}
		}

		auto newMessages = FetchFullMessages (messages);

		emit gotMsgHeaders (newMessages);
	}

	namespace
	{
		vmime::net::folder::path Folder2Path (const QStringList& folder)
		{
			if (folder.isEmpty ())
				return vmime::net::folder::path ("INBOX");

			vmime::net::folder::path path;
			Q_FOREACH (const auto& comp, folder)
				path.appendComponent (vmime::word (comp.toUtf8 ().constData (), vmime::charsets::UTF_8));
			return path;
		}
	}

	void AccountThreadWorker::FetchMessagesIMAP (Account::FetchFlags fetchFlags,
			const QList<QStringList>& origFolders, vmime::ref<vmime::net::store> store)
	{
		FetchMessagesInFolder (QStringList ("INBOX"), store->getDefaultFolder ());

		Q_FOREACH (const auto& folder, origFolders)
		{
			if (folder == QStringList ("INBOX"))
				continue;

			auto netFolder = store->getFolder (Folder2Path (folder));
			FetchMessagesInFolder (folder, netFolder);
		}
	}

	void AccountThreadWorker::FetchMessagesInFolder (const QStringList& folderName,
			vmime::utility::ref<vmime::net::folder> folder)
	{
		folder->open (vmime::net::folder::MODE_READ_WRITE);
		auto messages = folder->getMessages ();
		if (!messages.size ())
			return;

		const int desiredFlags = vmime::net::folder::FETCH_FLAGS |
					vmime::net::folder::FETCH_SIZE |
					vmime::net::folder::FETCH_UID |
					vmime::net::folder::FETCH_ENVELOPE;

		try
		{
			const auto& context = tr ("Fetching headers for %1")
					.arg (A_->GetName ());

			folder->fetchMessages (messages,
					desiredFlags, MkPgListener (context));
		}
		catch (const vmime::exceptions::operation_not_supported& ons)
		{
			qWarning () << Q_FUNC_INFO
					<< "fetch operation not supported:"
					<< ons.what ();
			return;
		}

		const QSet<QByteArray>& existing = Core::Instance ().GetStorage ()->LoadIDs (A_);

		QList<Message_ptr> newMessages;
		std::transform (messages.begin (), messages.end (), std::back_inserter (newMessages),
				[this, &folderName] (decltype (messages.front ()) msg)
				{
					auto res = FromHeaders (msg);
					res->AddFolder (folderName);
					return res;
				});

		QList<Message_ptr> updatedMessages;
		Q_FOREACH (Message_ptr msg, newMessages)
		{
			if (!existing.contains (msg->GetID ()))
				continue;

			newMessages.removeAll (msg);

			bool isUpdated = false;

			auto updated = Core::Instance ().GetStorage ()->
					LoadMessage (A_, msg->GetID ());

			if (updated->IsRead () != msg->IsRead ())
			{
				updated->SetRead (msg->IsRead ());
				isUpdated = true;
			}

			auto sumFolders = updated->GetFolders ();
			if (!folderName.isEmpty () &&
					!sumFolders.contains (folderName))
			{
				updated->AddFolder (folderName);
				isUpdated = true;
			}

			if (isUpdated)
				updatedMessages << updated;
		}

		emit gotMsgHeaders (newMessages);
		emit gotUpdatedMessages (updatedMessages);
	}

	namespace
	{
		void FullifyHeaderMessage (Message_ptr msg, const vmime::ref<vmime::message>& full)
		{
			vmime::messageParser mp (full);

			QString html;
			QString plain;

			Q_FOREACH (auto tp, mp.getTextPartList ())
			{
				if (tp->getType ().getType () != vmime::mediaTypes::TEXT)
				{
					qWarning () << Q_FUNC_INFO
							<< "non-text in text part"
							<< tp->getType ().getType ().c_str ();
					continue;
				}

				if (tp->getType ().getSubType () == vmime::mediaTypes::TEXT_HTML)
				{
					auto htp = tp.dynamicCast<const vmime::htmlTextPart> ();
					html = Stringize (htp->getText (), htp->getCharset ());
					plain = Stringize (htp->getPlainText (), htp->getCharset ());
				}
				else if (plain.isEmpty () &&
						tp->getType ().getSubType () == vmime::mediaTypes::TEXT_PLAIN)
					plain = Stringize (tp->getText (), tp->getCharset ());
			}

			msg->SetBody (plain);
			msg->SetHTMLBody (html);

			Q_FOREACH (auto att, mp.getAttachmentList ())
			{
				const auto& type = att->getType ();
				if (type.getType () == vmime::mediaTypes::TEXT &&
						(type.getSubType () == vmime::mediaTypes::TEXT_HTML ||
						 type.getSubType () == vmime::mediaTypes::TEXT_PLAIN))
					continue;

				msg->AddAttachment (att);
			}
		}
	}

	void AccountThreadWorker::SyncIMAPFolders (vmime::ref<vmime::net::store> store)
	{
		auto root = store->getRootFolder ();

		QList<QStringList> paths;

		auto folders = root->getFolders (true);
		Q_FOREACH (vmime::ref<vmime::net::folder> folder, root->getFolders (true))
		{
			QStringList pathList;
			const auto& path = folder->getFullPath ();
			for (int i = 0; i < path.getSize (); ++i)
				pathList << StringizeCT (path.getComponentAt (i));

			paths << pathList;
		}

		emit gotFolders (paths);
	}

	QList<Message_ptr> AccountThreadWorker::FetchFullMessages (const std::vector<vmime::utility::ref<vmime::net::message>>& messages)
	{
		const auto& context = tr ("Fetching messages for %1")
					.arg (A_->GetName ());

		auto pl = MkPgListener (context);

		QMetaObject::invokeMethod (pl,
				"start",
				Q_ARG (const int, messages.size ()));

		int i = 0;
		QList<Message_ptr> newMessages;
		Q_FOREACH (auto message, messages)
		{
			QMetaObject::invokeMethod (pl,
					"progress",
					Q_ARG (const int, ++i),
					Q_ARG (const int, messages.size ()));

			auto msgObj = FromHeaders (message);

			FullifyHeaderMessage (msgObj, FromNetMessage (message));

			newMessages << msgObj;
		}

		QMetaObject::invokeMethod (pl,
					"stop",
					Q_ARG (const int, messages.size ()));

		return newMessages;
	}

	ProgressListener* AccountThreadWorker::MkPgListener (const QString& text)
	{
		auto pl = new ProgressListener (text);
		pl->deleteLater ();
		emit gotProgressListener (ProgressListener_g_ptr (pl));
		return pl;
	}

	void AccountThreadWorker::synchronize (Account::FetchFlags flags, const QList<QStringList>& folders)
	{
		switch (A_->InType_)
		{
		case Account::InType::POP3:
			FetchMessagesPOP3 (flags);
			break;
		case Account::InType::IMAP:
		{
			TimerGuard g (DisconnectTimer_);

			auto store = MakeStore ();
			FetchMessagesIMAP (flags, folders, store);
			SyncIMAPFolders (store);
			break;
		}
		case Account::InType::Maildir:
			break;
		}
	}

	void AccountThreadWorker::fetchWholeMessage (Message_ptr origMsg)
	{
		if (!origMsg)
			return;

		if (A_->InType_ == Account::InType::POP3)
			return;

		TimerGuard g (DisconnectTimer_);

		const QByteArray& sid = origMsg->GetID ();
		vmime::string id (sid.constData ());
		qDebug () << Q_FUNC_INFO << sid.toHex ();

		auto store = MakeStore ();

		auto folder = store->getFolder (Folder2Path (origMsg->GetFolders ().value (0)));
		folder->open (vmime::net::folder::MODE_READ_WRITE);

		auto messages = folder->getMessages ();
		folder->fetchMessages (messages, vmime::net::folder::FETCH_UID);

		auto pos = std::find_if (messages.begin (), messages.end (),
				[id] (const vmime::ref<vmime::net::message>& message) { return message->getUniqueId () == id; });
		if (pos == messages.end ())
		{
			Q_FOREACH (auto msg, messages)
				qWarning () << QByteArray (msg->getUniqueId ().c_str ()).toHex ();
			qWarning () << Q_FUNC_INFO
					<< "message with ID"
					<< sid.toHex ()
					<< "not found in"
					<< messages.size ();
			return;
		}

		qDebug () << "found corresponding message, fullifying...";

		folder->fetchMessage (*pos, vmime::net::folder::FETCH_FLAGS |
					vmime::net::folder::FETCH_UID |
					vmime::net::folder::FETCH_CONTENT_INFO |
					vmime::net::folder::FETCH_STRUCTURE |
					vmime::net::folder::FETCH_FULL_HEADER);

		FullifyHeaderMessage (origMsg, FromNetMessage (*pos));

		qDebug () << "done";

		emit messageBodyFetched (origMsg);
	}

	void AccountThreadWorker::fetchAttachment (Message_ptr msg,
			const QString& attName, const QString& path)
	{
		if (A_->InType_ == Account::InType::POP3)
			return;

		TimerGuard g (DisconnectTimer_);

		const auto& msgId = msg->GetID ();
		vmime::string id (msgId.constData ());
		qDebug () << Q_FUNC_INFO << msgId.toHex ();

		auto store = MakeStore ();

		auto folder = store->getFolder (Folder2Path (msg->GetFolders ().value (0)));
		folder->open (vmime::net::folder::MODE_READ_WRITE);

		auto messages = folder->getMessages ();
		folder->fetchMessages (messages, vmime::net::folder::FETCH_UID);

		auto pos = std::find_if (messages.begin (), messages.end (),
				[id] (const vmime::ref<vmime::net::message>& message) { return message->getUniqueId () == id; });
		if (pos == messages.end ())
		{
			Q_FOREACH (auto msg, messages)
				qWarning () << QByteArray (msg->getUniqueId ().c_str ()).toHex ();
			qWarning () << Q_FUNC_INFO
					<< "message with ID"
					<< msgId.toHex ()
					<< "not found in"
					<< messages.size ();
			return;
		}

		vmime::messageParser mp ((*pos)->getParsedMessage ());
		Q_FOREACH (const vmime::ref<const vmime::attachment>& att, mp.getAttachmentList ())
		{
			if (StringizeCT (att->getName ()) != attName)
				continue;

			auto data = att->getData ();

			QFile file (path);
			if (!file.open (QIODevice::WriteOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open"
						<< path
						<< file.errorString ();
				return;
			}

			OutputIODevAdapter adapter (&file);
			data->extract (adapter,
					MkPgListener (tr ("Fetching attachment %1...").arg (attName)));

			const auto& e = Util::MakeNotification ("Snails",
					tr ("Attachment %1 fetched successfully.")
						.arg (attName),
					PInfo_);
			emit gotEntity (e);

			break;
		}
	}

	namespace
	{
		vmime::mailbox FromPair (const QString& name, const QString& email)
		{
			return vmime::mailbox (vmime::text (name.toUtf8 ().constData ()),
					email.toUtf8 ().constData ());
		}

		vmime::mailbox FromPair (const QPair<QString, QString>& pair)
		{
			return FromPair (pair.first, pair.second);
		}
	}

	void AccountThreadWorker::sendMessage (Message_ptr msg)
	{
		if (!msg)
			return;

		TimerGuard g (DisconnectTimer_);

		vmime::messageBuilder mb;
		mb.setSubject (vmime::text (msg->GetSubject ().toUtf8 ().constData ()));
		mb.setExpeditor (FromPair (msg->GetFrom (), msg->GetFromEmail ()));

		vmime::addressList recips;
		const auto& tos = msg->GetTo ();
		std::for_each (tos.begin (), tos.end (),
				[&recips] (decltype (tos.front ()) pair) { recips.appendAddress (vmime::create<vmime::mailbox> (FromPair (pair))); });
		mb.setRecipients (recips);

		mb.getTextPart ()->setCharset (vmime::charsets::UTF_8);
		mb.getTextPart ()->setText (vmime::create<vmime::stringContentHandler> (msg->GetBody ().toUtf8 ().constData ()));

		auto vMsg = mb.construct ();
		const auto& userAgent = QString ("LeechCraft Snails %1")
				.arg (Core::Instance ().GetProxy ()->GetVersion ());
		vMsg->getHeader ()->UserAgent ()->setValue (userAgent.toUtf8 ().constData ());

		auto pl = MkPgListener (tr ("Sending message %1...").arg (msg->GetSubject ()));
		auto transport = MakeTransport ();
		try
		{
			transport->connect ();
		}
		catch (const vmime::exceptions::authentication_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "authentication error:"
					<< e.what ()
					<< "with response"
					<< e.response ().c_str ();
			return;
		}
		transport->send (vMsg, pl);
	}

	void AccountThreadWorker::timeoutDisconnect ()
	{
		CachedStore_->disconnect ();
		CachedStore_ = vmime::ref<vmime::net::store> ();
	}
}
}
