/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountthreadworker.h"
#include <algorithm>
#include <QMutexLocker>
#include <QUrl>
#include <QFile>
#include <QtDebug>
#include <QTimer>
#include <QEventLoop>
#include <QCryptographicHash>
#include <vmime/security/defaultAuthenticator.hpp>
#include <vmime/net/transport.hpp>
#include <vmime/net/store.hpp>
#include <vmime/net/message.hpp>
#include <vmime/utility/datetimeUtils.hpp>
#include <vmime/dateTime.hpp>
#include <vmime/messageParser.hpp>
#include <vmime/messageBuilder.hpp>
#include <vmime/htmlTextPart.hpp>
#include <vmime/stringContentHandler.hpp>
#include <vmime/fileAttachment.hpp>
#include <vmime/messageIdSequence.hpp>
#include <vmime/attachmentHelper.hpp>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/containerconversions.h>
#include <util/sll/prelude.h>
#include <util/sll/unreachable.h>
#include "account.h"
#include "core.h"
#include "progresslistener.h"
#include "storage.h"
#include "accountdatabase.h"
#include "vmimeconversions.h"
#include "outputiodevadapter.h"
#include "common.h"
#include "messagechangelistener.h"
#include "folder.h"
#include "tracerfactory.h"
#include "accountthreadnotifier.h"
#include "certificateverifier.h"
#include "tracebytecounter.h"
#include "outgoingmessage.h"
#include "messagebodies.h"

namespace LC
{
namespace Snails
{
	namespace
	{
		template<typename T>
		auto WaitForFuture (const QFuture<T>& future)
		{
			QFutureWatcher<T> watcher;
			QEventLoop loop;

			QObject::connect (&watcher,
					&QFutureWatcher<T>::finished,
					&loop,
					&QEventLoop::quit);

			watcher.setFuture (future);

			loop.exec ();

			return future.result ();
		}

		class VMimeAuth : public vmime::security::defaultAuthenticator
		{
			Account::Direction Dir_;
			Account *Acc_;
		public:
			VMimeAuth (Account::Direction, Account*);

			const vmime::string getUsername () const override;
			const vmime::string getPassword () const override;
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
			const auto& cfg = Acc_->GetConfig ();
			switch (Dir_)
			{
			case Account::Direction::Out:
				return cfg.OutLogin_.toStdString ();
			case Account::Direction::In:
				return cfg.Login_.toStdString ();
			}

			Util::Unreachable ();
		}

		const vmime::string VMimeAuth::getPassword () const
		{
			return WaitForFuture (Acc_->GetPassword (Account::Direction::In)).toStdString ();
		}
	}

	const char* FolderNotFound::what () const
	{
		return "folder not found";
	}

	const char* MessageNotFound::what () const
	{
		return "message not found";
	}

	const char* FileOpenError::what () const
	{
		return "file open error";
	}

	const char* AttachmentNotFound::what () const
	{
		return "attachment not found";
	}

	const char* FolderAlreadyExists::what () const
	{
		return "folder already exists";
	}

	const char* InvalidPathComponent::what () const
	{
		return FullMessage_.constData ();
	}

	AccountThreadWorker::AccountThreadWorker (bool isListening,
			const QString& threadName, Account *parent, Storage *st)
	: A_ (parent)
	, Storage_ (st)
	, NoopTimer_ (new QTimer (this))
	, IsListening_ (isListening)
	, ThreadName_ (threadName)
	, ChangeListener_ (new MessageChangeListener (this))
	, Session_ (vmime::net::session::create ())
	, CachedFolders_ (2)
	, CertVerifier_ (vmime::make_shared<CertificateVerifier> ())
	, InAuth_ (vmime::make_shared<VMimeAuth> (Account::Direction::In, A_))
	{

		if (IsListening_)
			connect (ChangeListener_,
					&MessageChangeListener::messagesChanged,
					[] (const QStringList& folder, const QList<size_t>& numbers)
					{
						qDebug () << Q_FUNC_INFO << folder << numbers;
						auto set = vmime::net::messageSet::empty ();
						for (const auto& num : numbers)
							set.addRange (vmime::net::numberMessageRange { num });
					});

		connect (NoopTimer_,
				&QTimer::timeout,
				[this] { SendNoop (); });
		NoopTimer_->start (60 * 1000);
	}

	namespace
	{
		auto getFolderTracer (const VmimeFolder_ptr& folder)
		{
			const auto& store = folder->getStore ();
			const auto& tracer = store->getTracerFactory ();
			return vmime::dynamic_pointer_cast<TracerFactory> (tracer);
		}
	}

	vmime::shared_ptr<vmime::net::store> AccountThreadWorker::MakeStore ()
	{
		if (CachedStore_)
			return CachedStore_;

		auto url = WaitForFuture (A_->BuildInURL ());
		const auto& cfg = A_->GetConfig ();

		auto st = Session_->getStore (vmime::utility::url (url.toUtf8 ().constData ()));

		st->setTracerFactory (vmime::make_shared<TracerFactory> (ThreadName_, A_->GetLogger ()));

		st->setCertificateVerifier (CertVerifier_);
		st->setAuthenticator (InAuth_);

		if (cfg.InSecurity_ == AccountConfig::SecurityType::TLS)
		{
			st->setProperty ("connection.tls", true);
			st->setProperty ("connection.tls.required", cfg.InSecurityRequired_);
		}
		st->setProperty ("options.sasl", cfg.UseSASL_);
		st->setProperty ("options.sasl.fallback", cfg.SASLRequired_);
		st->setProperty ("server.port", cfg.InPort_);

		st->connect ();

		if (IsListening_)
			if (const auto defFolder = st->getDefaultFolder ())
			{
				defFolder->addMessageChangedListener (ChangeListener_);
				CachedFolders_ [GetFolderPath (defFolder)] = defFolder;
			}

		CachedStore_ = st;

		return st;
	}

	vmime::shared_ptr<vmime::net::transport> AccountThreadWorker::MakeTransport ()
	{
		auto url = WaitForFuture (A_->BuildOutURL ());
		const auto& cfg = A_->GetConfig ();

		QString username;
		QString password;
		bool setAuth = false;
		if (cfg.SMTPNeedsAuth_ &&
				cfg.OutType_ == AccountConfig::OutType::SMTP)
		{
			setAuth = true;

			QUrl parsed = QUrl::fromEncoded (url.toUtf8 ());
			username = parsed.userName ();
			password = parsed.password ();

			parsed.setUserName (QString ());
			parsed.setPassword (QString ());
			url = QString::fromUtf8 (parsed.toEncoded ());
			qDebug () << Q_FUNC_INFO << url << username << password;
		}

		auto trp = Session_->getTransport (vmime::utility::url (url.toUtf8 ().constData ()));
		trp->setCertificateVerifier (CertVerifier_);

		if (setAuth)
		{
			trp->setProperty ("options.need-authentication", true);
			trp->setProperty ("auth.username", username.toUtf8 ().constData ());
			trp->setProperty ("auth.password", password.toUtf8 ().constData ());
		}
		trp->setProperty ("server.port", cfg.OutPort_);

		if (cfg.OutSecurity_ == AccountConfig::SecurityType::TLS)
		{
			trp->setProperty ("connection.tls", true);
			trp->setProperty ("connection.tls.required", cfg.OutSecurityRequired_);
		}

		return trp;
	}

	namespace
	{
		vmime::net::folder::path Folder2Path (const QStringList& folder)
		{
			if (folder.isEmpty ())
				return vmime::net::folder::path ("INBOX");

			vmime::net::folder::path path;
			for (const auto& comp : folder)
				path.appendComponent ({ comp.toUtf8 ().constData (), vmime::charsets::UTF_8 });
			return path;
		}
	}

	VmimeFolder_ptr AccountThreadWorker::GetFolder (const QStringList& path, FolderMode mode)
	{
		if (path.size () == 1 && path.at (0) == "[Gmail]")
			return {};

		if (!CachedFolders_.contains (path))
		{
			auto store = MakeStore ();
			CachedFolders_ [path] = store->getFolder (Folder2Path (path));
		}

		auto folder = CachedFolders_ [path];

		if (mode == FolderMode::NoChange)
			return folder;

		const auto requestedMode = mode == FolderMode::ReadOnly ?
				vmime::net::folder::MODE_READ_ONLY :
				vmime::net::folder::MODE_READ_WRITE;

		const auto isOpen = folder->isOpen ();
		if (isOpen && folder->getMode () == requestedMode)
			return folder;
		else if (isOpen)
			folder->close (false);

		folder->open (requestedMode);

		return folder;
	}

	namespace
	{
		template<typename F>
		class MessageStructHandler
		{
			F Handler_;
		public:
			MessageStructHandler (const vmime::shared_ptr<vmime::net::message>& msg, const F& handler)
			: Handler_ { handler }
			{
				if (const auto& structure = msg->getStructure ())
					EnumParts (structure);
			}
		private:
			void HandlePart (const vmime::shared_ptr<vmime::net::messagePart>& part)
			{
				const auto& type = part->getType ();

				if (type.getType () == "text" &&
						(part->getDisposition ().getName () == "inline" || part->getName ().empty ()))
					return;

				if (type.getType () == "multipart")
				{
					EnumParts (part);
					return;
				}

				Handler_ (part);
			}

			template<typename T>
			void EnumParts (const T& enumable)
			{
				const auto pc = enumable->getPartCount ();
				for (vmime::size_t i = 0; i < pc; ++i)
					HandlePart (enumable->getPartAt (i));
			}
		};

		QList<AttDescr> CollectAttachments (const vmime::shared_ptr<vmime::net::message>& message)
		{
			QList<AttDescr> atts;
			MessageStructHandler
			{
				message,
				[&atts] (const auto& part)
				{
					const auto& type = part->getType ();
					atts << AttDescr {
							QString::fromStdString (part->getName ()),
							{},
							type.getType ().c_str (),
							type.getSubType ().c_str (),
							static_cast<qlonglong> (part->getSize ())
						};
				}
			};
			return atts;
		}
	}

	FetchedMessageInfo AccountThreadWorker::FromHeaders (const vmime::shared_ptr<vmime::net::message>& message) const
	{
		const auto& utf8cs = vmime::charset { vmime::charsets::UTF_8 };

		MessageInfo msg;
		msg.FolderId_ = static_cast<vmime::string> (message->getUID ()).c_str ();
		msg.Size_ = message->getSize ();
		msg.IsRead_ = message->getFlags () & vmime::net::message::FLAG_SEEN;

		auto header = message->getHeader ();

		if (const auto& from = header->From ())
		{
			const auto& mboxVal = from->getValue ();
			const auto& mbox = vmime::dynamicCast<const vmime::mailbox> (mboxVal);
			msg.Addresses_ [AddressType::From] << Mailbox2Strings (mbox);
		}
		else
			qWarning () << "no 'from' data";

		if (const auto& msgIdField = header->MessageId ())
		{
			const auto& msgIdVal = msgIdField->getValue ();
			const auto& msgId = vmime::dynamicCast<const vmime::messageId> (msgIdVal);
			msg.MessageId_ = msgId->getId ().c_str ();
		}

		if (const auto& refHeader = header->References ())
		{
			const auto& seqVal = refHeader->getValue ();
			const auto& seq = vmime::dynamicCast<const vmime::messageIdSequence> (seqVal);

			for (const auto& id : seq->getMessageIdList ())
				msg.References_ << id->getId ().c_str ();
		}

		if (const auto& irt = header->InReplyTo ())
		{
			const auto& seqVal = irt->getValue ();
			const auto& seq = vmime::dynamicCast<const vmime::messageIdSequence> (seqVal);

			for (const auto& id : seq->getMessageIdList ())
				msg.InReplyTo_ << id->getId ().c_str ();
		}

		auto setAddresses = [&msg] (AddressType type, const vmime::shared_ptr<const vmime::headerField>& field)
		{
			if (!field)
				return;

			const auto fieldVal = field->getValue ();
			if (const auto& alist = vmime::dynamicCast<const vmime::addressList> (fieldVal))
				msg.Addresses_ [type] = Util::Map (alist->toMailboxList ()->getMailboxList (), &Mailbox2Strings);
			else if (const auto& mbox = vmime::dynamicCast<const vmime::mailbox> (fieldVal))
				msg.Addresses_ [type] << Mailbox2Strings (mbox);
			else
			{
				const auto fieldPtr = field.get ();
				qWarning () << "no"
						<< static_cast<int> (type)
						<< "data: cannot cast to a mailbox list or a mailbox"
						<< typeid (*fieldPtr).name ();
			}
		};

		setAddresses (AddressType::To, header->To ());
		setAddresses (AddressType::Cc, header->Cc ());
		setAddresses (AddressType::Bcc, header->Bcc ());
		setAddresses (AddressType::ReplyTo, header->ReplyTo ());
		if (const auto dateHeader = header->Date ())
		{
			const auto& origDateVal = dateHeader->getValue ();
			const auto& origDate = vmime::dynamicCast<const vmime::datetime> (origDateVal);
			const auto& date = vmime::utility::datetimeUtils::toUniversalTime (*origDate);
			QDate qdate (date.getYear (), date.getMonth (), date.getDay ());
			QTime time (date.getHour (), date.getMinute (), date.getSecond ());
			msg.Date_ = { qdate, time, Qt::UTC };
		}
		else
			qWarning () << "no 'date' data";

		if (const auto subjectHeader = header->Subject ())
		{
			const auto& strVal = subjectHeader->getValue ();
			const auto& str = vmime::dynamicCast<const vmime::text> (strVal);
			msg.Subject_ = QString::fromStdString (str->getConvertedText (utf8cs));
		}
		else
			qWarning () << "no 'subject' data";

		msg.Attachments_ = CollectAttachments (message);

		return { msg, header };
	}

	namespace
	{
		template<typename F, typename D>
		auto TryOrDie (D&& disconnect, F&& func, int retries = 3)
		{
			try
			{
				return std::invoke (std::forward<F> (func));
			}
			catch (const std::exception& e)
			{
				std::invoke (std::forward<D> (disconnect));

				if (retries)
					return TryOrDie (std::forward<D> (disconnect), std::forward<F> (func), --retries);
				else
					throw;
			}
		}
	}

	auto AccountThreadWorker::SyncMessagesStatuses (const QStringList& folderName) -> SyncStatusesResult
	{
		return TryOrDie ([this] { Disconnect (); },
				[&]
				{
					auto store = MakeStore ();
					auto netFolder = store->getFolder (Folder2Path (folderName));
					netFolder->open (vmime::net::folder::MODE_READ_ONLY);

					return SyncMessagesStatusesImpl (folderName, netFolder);
				});
	}

	namespace
	{
		template<typename F>
		auto GetAllMessagesInFolder (const VmimeFolder_ptr& folder, int desiredFlags, F)
		{
			const auto& set = vmime::net::messageSet::byNumber (1, -1);
			return folder->getAndFetchMessages (set, desiredFlags);
		}

		template<typename F>
		MessageVector_t GetMessagesInFolder (const VmimeFolder_ptr& folder, const QByteArray& lastId, F progMaker)
		{
			const int desiredFlags = vmime::net::fetchAttributes::FLAGS |
						vmime::net::fetchAttributes::SIZE |
						vmime::net::fetchAttributes::UID |
						vmime::net::fetchAttributes::FULL_HEADER |
						vmime::net::fetchAttributes::STRUCTURE |
						vmime::net::fetchAttributes::ENVELOPE;

			if (lastId.isEmpty ())
				return GetAllMessagesInFolder (folder, desiredFlags, progMaker);

			const auto& set = vmime::net::messageSet::byUID (lastId.constData (), "*");
			try
			{
				return folder->getAndFetchMessages (set, desiredFlags);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot get messages from"
						<< lastId
						<< "because:"
						<< e.what ();
				throw;
			}
		}

		template<typename F>
		MessageVector_t GetAllMessageIdsInFolder (const VmimeFolder_ptr& folder, F progMaker)
		{
			const int desiredFlags = vmime::net::fetchAttributes::FLAGS;
			return GetAllMessagesInFolder (folder, desiredFlags, progMaker);
		}
	}

	auto AccountThreadWorker::FetchMessagesInFolder (const QStringList& folderName,
			const VmimeFolder_ptr& folder, const QByteArray& lastId) -> QList<FetchedMessageInfo>
	{
		const auto changeGuard = ChangeListener_->Disable ();

		const auto bytesCounter = getFolderTracer (folder)->CreateCounter ();

		qDebug () << Q_FUNC_INFO << folderName << folder.get () << lastId;

		auto messages = GetMessagesInFolder (folder, lastId,
				[this, folderName]
				{
					return A_->MakeProgressListener (tr ("Fetching messages in %1...")
							.arg (folderName.join ("/")));
				});

		qDebug () << "done fetching, sent" << bytesCounter.GetSent ()
				<< "bytes, received" << bytesCounter.GetReceived () << "bytes";

		auto newMessages = Util::Map (messages, [this, &folderName] (const auto& msg)
				{
					auto res = FromHeaders (msg);
					res.Info_.Folder_ = folderName;
					return res;
				});
		const auto& existing = Util::AsSet (Storage_->LoadIDs (A_, folderName));

		newMessages.erase (std::remove_if (newMessages.begin (), newMessages.end (),
					[&existing] (const auto& pair) { return existing.contains (pair.Info_.FolderId_); }),
				newMessages.end ());

		return newMessages;
	}

	auto AccountThreadWorker::SyncMessagesStatusesImpl (const QStringList& folderName,
			const VmimeFolder_ptr& folder) -> SyncStatusesResult
	{
		const auto bytesCounter = getFolderTracer (folder)->CreateCounter ();

		qDebug () << Q_FUNC_INFO << folderName << folder.get ();

		auto remoteIds = GetAllMessageIdsInFolder (folder,
				[this, folderName]
				{
					return A_->MakeProgressListener (tr ("Fetching message IDs in %1...")
							.arg (folderName.join ("/")));
				});

		qDebug () << "done fetching, sent" << bytesCounter.GetSent ()
				<< "bytes, received" << bytesCounter.GetReceived () << "bytes";

		const auto base = Storage_->BaseForAccount (A_);
		auto localIds = Util::AsSet (base->GetIDs (folderName));

		SyncStatusesResult result;
		for (const auto& msg : remoteIds)
		{
			const auto& uid = QByteArray::fromStdString (msg->getUID ());
			if (!localIds.remove (uid))
				continue;

			const auto isStoredRead = base->IsMessageRead (uid, folderName).value ();
			const auto isRemoteRead = msg->getFlags () & vmime::net::message::FLAG_SEEN;

			if (isStoredRead != isRemoteRead)
			{
				auto& list = isRemoteRead ? result.RemoteBecameRead_ : result.RemoteBecameUnread_;
				list << uid;
			}
		}
		result.RemovedIds_ = localIds.values ();
		return result;
	}

	namespace
	{
		MessageBodies GetMessageBodies (const vmime::shared_ptr<vmime::net::message>& full)
		{
			vmime::messageParser mp { full->getParsedMessage () };

			QString html;
			QStringList plainParts;
			QStringList htmlPlainParts;

			for (const auto& tp : mp.getTextPartList ())
			{
				const auto& type = tp->getType ();
				if (type.getType () != vmime::mediaTypes::TEXT)
				{
					qWarning () << Q_FUNC_INFO
							<< "non-text in text part"
							<< tp->getType ().getType ().c_str ();
					continue;
				}

				if (type.getSubType () == vmime::mediaTypes::TEXT_HTML)
				{
					auto htp = vmime::dynamicCast<const vmime::htmlTextPart> (tp);
					html = Stringize (htp->getText (), htp->getCharset ());
					htmlPlainParts << Stringize (htp->getPlainText (), htp->getCharset ());
				}
				else if (type.getSubType () == vmime::mediaTypes::TEXT_PLAIN)
					plainParts << Stringize (tp->getText (), tp->getCharset ());
			}

			if (plainParts.isEmpty ())
				plainParts = htmlPlainParts;

			if (std::adjacent_find (plainParts.begin (), plainParts.end (),
					[] (const QString& left, const QString& right)
						{ return left.size () > right.size (); }) == plainParts.end ())
				std::reverse (plainParts.begin (), plainParts.end ());

			return { plainParts.join ("\n"), html };
		}

		FolderType ToFolderType (int specialUse)
		{
			switch (static_cast<vmime::net::folderAttributes::SpecialUses> (specialUse))
			{
			case vmime::net::folderAttributes::SPECIALUSE_SENT:
				return FolderType::Sent;
			case vmime::net::folderAttributes::SPECIALUSE_DRAFTS:
				return FolderType::Drafts;
			case vmime::net::folderAttributes::SPECIALUSE_IMPORTANT:
				return FolderType::Important;
			case vmime::net::folderAttributes::SPECIALUSE_JUNK:
				return FolderType::Junk;
			case vmime::net::folderAttributes::SPECIALUSE_TRASH:
				return FolderType::Trash;
			default:
				return FolderType::Other;
			}
		}
	}

	void AccountThreadWorker::SetNoopTimeout (int timeout)
	{
		NoopTimer_->stop ();

		if (timeout > NoopTimer_->interval ())
			SendNoop ();

		NoopTimer_->start (timeout);
	}

	void AccountThreadWorker::SendNoop ()
	{
		if (CachedStore_)
			CachedStore_->noop ();
	}

	void AccountThreadWorker::SetNoopTimeoutChangeNotifier (const std::shared_ptr<AccountThreadNotifier<int>>& notifier)
	{
		SetNoopTimeout (notifier->GetData ());

		connect (notifier.get (),
				&AccountThreadNotifier<int>::changed,
				[this, notifier] { SetNoopTimeout (notifier->GetData ()); });
	}

	void AccountThreadWorker::Disconnect ()
	{
		CachedFolders_.clear ();

		if (!CachedStore_)
			return;

		if (CachedStore_->isConnected ())
			CachedStore_->disconnect ();

		CachedStore_.reset ();
	}

	void AccountThreadWorker::TestConnectivity ()
	{
		if (!CachedStore_)
			MakeStore ();

		SendNoop ();
	}

	QList<Folder> AccountThreadWorker::SyncFolders ()
	{
		auto store = MakeStore ();

		const auto& root = store->getRootFolder ();
		const auto& inbox = store->getDefaultFolder ();

		QList<Folder> folders;
		for (const auto& folder : root->getFolders (true))
		{
			const auto& attrs = folder->getAttributes ();
			folders.append ({
					GetFolderPath (folder),
					folder->getFullPath () == inbox->getFullPath () ?
						FolderType::Inbox :
						ToFolderType (attrs.getSpecialUse ())
				});
		}
		return folders;
	}

	auto AccountThreadWorker::Synchronize (const QList<QStringList>& foldersToFetch, const QByteArray& last) -> SyncResult
	{
		Folder2Messages_t result;

		const auto pl = A_->MakeProgressListener (tr ("Synchronizing messages..."));
		pl->start (foldersToFetch.size ());

		for (const auto& folder : foldersToFetch)
		{
			TryOrDie ([this] { Disconnect (); },
					[&]
					{
						if (const auto& netFolder = GetFolder (folder, FolderMode::ReadOnly))
							result [folder] = FetchMessagesInFolder (folder, netFolder, last);
					});

			pl->Increment ();
		}

		pl->stop (foldersToFetch.size ());

		return { result };
	}

	auto AccountThreadWorker::GetMessageCount (const QStringList& folder) -> MsgCountResult_t
	{
		auto netFolder = GetFolder (folder, FolderMode::NoChange);
		if (!netFolder)
			return MsgCountResult_t::Left (FolderNotFound {});

		const auto& status = netFolder->getStatus ();
		auto count = status->getMessageCount ();
		if (count >= 3000)
			count = GetFolder (folder, FolderMode::ReadOnly)->getMessageCount ();
		return MsgCountResult_t::Right ({ count, status->getUnseenCount () });
	}

	auto AccountThreadWorker::SetReadStatus (bool read,
			const QList<QByteArray>& ids, const QStringList& folderPath) -> SetReadStatusResult_t
	{
		const auto& folder = GetFolder (folderPath, FolderMode::ReadWrite);
		if (!folder)
			return SetReadStatusResult_t::Left (FolderNotFound {});

		folder->setMessageFlags (ToMessageSet (ids),
				vmime::net::message::Flags::FLAG_SEEN,
				read ?
						vmime::net::message::FLAG_MODE_ADD :
						vmime::net::message::FLAG_MODE_REMOVE);

		return SetReadStatusResult_t::Right ({});
	}

	FetchWholeMessageResult_t AccountThreadWorker::FetchWholeMessage (const QStringList& folderId, const QByteArray& msgId)
	{
		qDebug () << Q_FUNC_INFO << msgId;
		auto folder = GetFolder (folderId, FolderMode::ReadOnly);
		if (!folder)
			return FetchWholeMessageResult_t::Left (FolderNotFound {});

		const auto& set = vmime::net::messageSet::byUID (msgId.constData ());
		const auto attrs = vmime::net::fetchAttributes::FLAGS |
				vmime::net::fetchAttributes::UID |
				vmime::net::fetchAttributes::CONTENT_INFO |
				vmime::net::fetchAttributes::STRUCTURE |
				vmime::net::fetchAttributes::FULL_HEADER;
		const auto& messages = folder->getAndFetchMessages (set, attrs);
		if (messages.empty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "message with ID"
					<< msgId
					<< "not found in"
					<< messages.size ();
			return FetchWholeMessageResult_t::Left (MessageNotFound {});
		}

		auto bodies = GetMessageBodies (messages.front ());
		qDebug () << Q_FUNC_INFO << "done";
		return FetchWholeMessageResult_t::Right (std::move (bodies));
	}

	PrefetchWholeMessagesResult_t AccountThreadWorker::PrefetchWholeMessages (const QStringList& folderId,
			const QList<QByteArray>& msgIds)
	{
		qDebug () << Q_FUNC_INFO << msgIds.size ();
		auto folder = GetFolder (folderId, FolderMode::ReadOnly);
		if (!folder)
			return PrefetchWholeMessagesResult_t::Left (FolderNotFound {});

		std::vector<vmime::net::message::uid> uids;
		uids.reserve (msgIds.size ());
		for (const auto& msgId : msgIds)
			uids.push_back (msgId.constData ());
		const auto& set = vmime::net::messageSet::byUID (uids);

		const auto attrs = vmime::net::fetchAttributes::FLAGS |
				vmime::net::fetchAttributes::UID |
				vmime::net::fetchAttributes::CONTENT_INFO |
				vmime::net::fetchAttributes::STRUCTURE |
				vmime::net::fetchAttributes::FULL_HEADER;
		const auto& messages = folder->getAndFetchMessages (set, attrs);

		QHash<QByteArray, MessageBodies> result;
		for (const auto& msg : messages)
			result [QByteArray::fromStdString (msg->getUID ())] = GetMessageBodies (msg);

		qDebug () << Q_FUNC_INFO << "done";

		return PrefetchWholeMessagesResult_t::Right (std::move (result));
	}

	FetchAttachmentResult_t AccountThreadWorker::FetchAttachment (const QStringList& folderId,
			const QByteArray& msgId, const QString& attName, const QString& path)
	{
		const auto& folder = GetFolder (folderId, FolderMode::ReadOnly);
		const auto& msgSet = vmime::net::messageSet::byUID (msgId.constData ());
		const auto& messages = folder->getAndFetchMessages (msgSet, vmime::net::fetchAttributes::STRUCTURE);
		if (messages.empty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "message with ID"
					<< msgId.toHex ()
					<< "not found in"
					<< messages.size ();
			return FetchAttachmentResult_t::Left (MessageNotFound {});
		}

		const auto& parsedMsg = messages.front ()->getParsedMessage ();
		const auto& attachments = vmime::attachmentHelper::findAttachmentsInMessage (parsedMsg,
				vmime::attachmentHelper::INLINE_OBJECTS);
		for (const auto& att : attachments)
		{
			if (StringizeCT (att->getName ()) != attName)
				continue;

			const auto& data = att->getData ();
			QFile file { path };
			if (!file.open (QIODevice::WriteOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open"
						<< path
						<< file.errorString ();
				return FetchAttachmentResult_t::Left (FileOpenError {});
			}

			OutputIODevAdapter adapter (&file);
			const auto pl = A_->MakeProgressListener (tr ("Fetching attachment %1...")
					.arg (attName));
			data->extract (adapter, pl.get ());

			return FetchAttachmentResult_t::Right ({});
		}

		qWarning () << Q_FUNC_INFO
				<< "attachment not found"
				<< attName;

		return FetchAttachmentResult_t::Left (AttachmentNotFound {});
	}

	void AccountThreadWorker::CopyMessages (const QList<QByteArray>& ids,
			const QStringList& from, const QList<QStringList>& tos)
	{
		if (ids.isEmpty () || tos.isEmpty ())
			return;

		const auto& folder = GetFolder (from, FolderMode::ReadWrite);
		if (!folder)
			return;

		const auto& set = ToMessageSet (ids);
		for (const auto& to : tos)
			folder->copyMessages (Folder2Path (to), set);
	}

	auto AccountThreadWorker::DeleteMessages (const QList<QByteArray>& ids,
			const QStringList& path) -> DeleteResult_t
	{
		if (ids.isEmpty ())
			return DeleteResult_t::Right ({});

		const auto& folder = GetFolder (path, FolderMode::ReadWrite);
		if (!folder)
		{
			qWarning () << Q_FUNC_INFO
					<< path
					<< "not found";
			return DeleteResult_t::Left (FolderNotFound {});
		}

		folder->deleteMessages (ToMessageSet (ids));
		folder->expunge ();

		return DeleteResult_t::Right ({});
	}

	namespace
	{
		vmime::shared_ptr<vmime::mailbox> FromAddress (const Address& addr)
		{
			return vmime::make_shared<vmime::mailbox> (vmime::text (addr.Name_.toUtf8 ().constData ()),
					addr.Email_.toUtf8 ().constData ());
		}

		vmime::addressList ToAddressList (const Addresses_t& addresses)
		{
			vmime::addressList result;
			for (const auto& pair : addresses)
				result.appendAddress (FromAddress (pair));
			return result;
		}

		vmime::messageIdSequence ToMessageIdSequence (const QList<QByteArray>& ids)
		{
			vmime::messageIdSequence result;
			for (const auto& id : ids)
				result.appendMessageId (vmime::make_shared<vmime::messageId> (id.constData ()));
			return result;
		}

		vmime::messageId GenerateMsgId (const OutgoingMessage& msg)
		{
			const auto& senderAddress = msg.From_.Email_;

			const auto& contents = msg.Body_.toUtf8 ();
			const auto& contentsHash = QCryptographicHash::hash (contents, QCryptographicHash::Sha1).toBase64 ();

			const auto& now = QDateTime::currentDateTime ();

			const auto& id = now.toString ("ddMMyyyy.hhmmsszzzap") + '%' + contentsHash + '%' + senderAddress;

			return vmime::string { id.toUtf8 ().constData () };
		}
	}

	void AccountThreadWorker::SendMessage (const OutgoingMessage& msg)
	{
		vmime::messageBuilder mb;
		mb.setSubject (vmime::text (msg.Subject_.toUtf8 ().constData ()));
		mb.setExpeditor (*FromAddress (msg.From_));
		mb.setRecipients (ToAddressList (msg.To_));
		mb.setCopyRecipients (ToAddressList (msg.Cc_));
		mb.setBlindCopyRecipients (ToAddressList (msg.Bcc_));

		const auto& html = msg.HTMLBody_;

		if (html.isEmpty ())
		{
			mb.getTextPart ()->setCharset (vmime::charsets::UTF_8);
			mb.getTextPart ()->setText (vmime::make_shared<vmime::stringContentHandler> (msg.Body_.toUtf8 ().constData ()));
		}
		else
		{
			mb.constructTextPart ({ vmime::mediaTypes::TEXT, vmime::mediaTypes::TEXT_HTML });
			const auto& textPart = vmime::dynamicCast<vmime::htmlTextPart> (mb.getTextPart ());
			textPart->setCharset (vmime::charsets::UTF_8);
			textPart->setText (vmime::make_shared<vmime::stringContentHandler> (html.toUtf8 ().constData ()));
			textPart->setPlainText (vmime::make_shared<vmime::stringContentHandler> (msg.Body_.toUtf8 ().constData ()));
		}

		for (const auto& descr : msg.Attachments_)
		{
			try
			{
				const QFileInfo fi (descr.GetName ());
				const auto& att = vmime::make_shared<vmime::fileAttachment> (descr.GetName ().toUtf8 ().constData (),
						vmime::mediaType (descr.GetType ().constData (), descr.GetSubType ().constData ()),
						vmime::text (descr.GetDescr ().toUtf8 ().constData ()));
				att->getFileInfo ().setFilename (fi.fileName ().toUtf8 ().constData ());
				att->getFileInfo ().setSize (descr.GetSize ());

				mb.appendAttachment (att);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "failed to append"
						<< descr.GetName ()
						<< e.what ();
			}
		}

		const auto& vMsg = mb.construct ();
		const auto& userAgent = QString ("LeechCraft Snails %1")
				.arg (Core::Instance ().GetProxy ()->GetVersion ());
		const auto& header = vMsg->getHeader ();
		header->UserAgent ()->setValue (userAgent.toUtf8 ().constData ());

		if (!msg.InReplyTo_.isEmpty ())
			header->InReplyTo ()->setValue (ToMessageIdSequence (msg.InReplyTo_));
		if (!msg.References_.isEmpty ())
			header->References ()->setValue (ToMessageIdSequence (msg.References_));

		header->MessageId ()->setValue (GenerateMsgId (msg));

		auto pl = A_->MakeProgressListener (tr ("Sending message %1...").arg (msg.Subject_));
		auto transport = MakeTransport ();
		try
		{
			if (!transport->isConnected ())
				transport->connect ();
		}
		catch (const vmime::exceptions::authentication_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "authentication error:"
					<< e.what ()
					<< "with response"
					<< e.response ().c_str ();
			throw;
		}
		transport->send (vMsg, pl.get ());
	}

	CreateFolderResult_t AccountThreadWorker::CreateFolder (const QStringList& folderPath)
	{
		auto store = MakeStore ();

		for (const auto& comp : folderPath)
			if (!store->isValidFolderName ({ comp.toUtf8 ().constData (), vmime::charsets::UTF_8 }))
				return CreateFolderResult_t::Left (InvalidPathComponent { comp });

		auto folder = store->getFolder (Folder2Path (folderPath));
		if (folder->exists ())
			return CreateFolderResult_t::Left (FolderAlreadyExists {});

		vmime::net::folderAttributes attrs;
		attrs.setType (vmime::net::folderAttributes::TYPE_CONTAINS_MESSAGES);
		folder->create (attrs);
		try
		{
			folder->create ({});
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
		}
		return CreateFolderResult_t::Right ({});
	}
}
}
