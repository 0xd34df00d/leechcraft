/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "messagelistactionsmanager.h"
#include <QUrl>
#include <QMessageBox>
#include <QTextDocument>
#include <QMenu>
#include <QtDebug>
#include <vmime/messageIdSequence.hpp>
#include <util/xpc/util.h>
#include <util/threads/futures.h>
#include <util/sll/urlaccessor.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "messageinfo.h"
#include "vmimeconversions.h"
#include "account.h"
#include "storage.h"
#include "accountdatabase.h"
#include "outgoingmessage.h"
#include "util.h"

namespace LC
{
namespace Snails
{
	using Header_ptr = vmime::shared_ptr<const vmime::header>;

	class MessageListActionsProvider
	{
	public:
		virtual QList<MessageListActionInfo> GetMessageActions (const MessageInfo&, const Header_ptr&, Account*) const = 0;
	};

	namespace
	{
		Header_ptr HeaderFromContents (const QByteArray& contents)
		{
			auto header = vmime::make_shared<vmime::header> ();
			header->parse ({ contents.constData (), static_cast<size_t> (contents.size ()) });
			return header;
		}

		vmime::shared_ptr<const vmime::messageId> GetGithubMsgId (const Header_ptr& headers)
		{
			const auto& referencesField = headers->References ();
			if (!referencesField)
			{
				if (const auto msgId = headers->MessageId ())
					return msgId->getValue<vmime::messageId> ();
				return {};
			}

			const auto& refSeq = referencesField->getValue<vmime::messageIdSequence> ();
			if (!refSeq)
				return {};

			if (!refSeq->getMessageIdCount ())
				return {};

			return refSeq->getMessageIdAt (0);
		}

		QString GetGithubAddr (const Header_ptr& headers)
		{
			if (!headers->findField ("X-GitHub-Sender"))
				return {};

			const auto& ref = GetGithubMsgId (headers);
			if (!ref)
				return {};

			return QString::fromStdString (ref->getLeft ());
		}

		class GithubProvider final : public MessageListActionsProvider
		{
		public:
			QList<MessageListActionInfo> GetMessageActions (const MessageInfo&, const Header_ptr& headers, Account*) const override
			{
				const auto& addrReq = GetGithubAddr (headers);
				if (addrReq.isEmpty ())
					return {};

				return
				{
					{
						QObject::tr ("Open"),
						QObject::tr ("Open the page on GitHub."),
						QIcon::fromTheme ("document-open"),
						QColor { "red" },
						MessageListActionFlag::None,
						[addrReq] (const MessageInfo&)
						{
							const QUrl fullUrl { "https://github.com/" + addrReq };
							const auto& entity = Util::MakeEntity (fullUrl, {}, FromUserInitiated | OnlyHandle);
							Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (entity);
						},
						{}
					}
				};
			}
		};

		class BugzillaProvider final : public MessageListActionsProvider
		{
		public:
			QList<MessageListActionInfo> GetMessageActions (const MessageInfo&, const Header_ptr& headers, Account*) const override
			{
				const auto header = headers->findField ("X-Bugzilla-URL");
				if (!header)
					return {};

				const auto& urlText = header->getValue<vmime::text> ();
				if (!urlText)
					return {};

				const auto& url = StringizeCT (*urlText);

				const auto& referencesField = headers->MessageId ();
				if (!referencesField)
					return {};

				const auto& ref = referencesField->getValue<vmime::messageId> ();
				if (!ref)
					return {};

				const auto& left = QString::fromStdString (ref->getLeft ());
				const auto bugId = left.section ('-', 1, 1);

				return
				{
					{
						QObject::tr ("Open"),
						QObject::tr ("Open the bug page on Bugzilla."),
						QIcon::fromTheme ("tools-report-bug"),
						QColor { "red" },
						MessageListActionFlag::None,
						[url, bugId] (const MessageInfo&)
						{
							const QUrl fullUrl { url + "show_bug.cgi?id=" + bugId };
							const auto& entity = Util::MakeEntity (fullUrl, {}, FromUserInitiated | OnlyHandle);
							Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (entity);
						},
						{}
					}
				};
			}
		};

		class RedmineProvider final : public MessageListActionsProvider
		{
		public:
			QList<MessageListActionInfo> GetMessageActions (const MessageInfo&, const Header_ptr& headers, Account*) const override
			{
				const auto header = headers->findField ("X-Redmine-Host");
				if (!header)
					return {};

				const auto& urlText = header->getValue<vmime::text> ();
				if (!urlText)
					return {};

				const auto& url = StringizeCT (*urlText);

				const auto issueHeader = headers->findField ("X-Redmine-Issue-Id");
				if (!issueHeader)
					return {};

				const auto& issueText = issueHeader->getValue<vmime::text> ();
				if (!issueText)
					return {};

				const auto& issue = StringizeCT (*issueText);

				return
				{
					{
						QObject::tr ("Open"),
						QObject::tr ("Open the issue page on Redmine."),
						QIcon::fromTheme ("tools-report-bug"),
						QColor { "red" },
						MessageListActionFlag::None,
						[url, issue] (const MessageInfo&)
						{
							const QUrl fullUrl { "http://" + url + "/issues/" + issue };
							const auto& entity = Util::MakeEntity (fullUrl, {}, FromUserInitiated | OnlyHandle);
							Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (entity);
						},
						{}
					}
				};
			}
		};

		class ReviewboardProvider final : public MessageListActionsProvider
		{
		public:
			QList<MessageListActionInfo> GetMessageActions (const MessageInfo&, const Header_ptr& headers, Account*) const override
			{
				const auto header = headers->findField ("X-ReviewRequest-URL");
				if (!header)
					return {};

				const auto& urlText = header->getValue<vmime::text> ();
				if (!urlText)
					return {};

				const auto& url = StringizeCT (*urlText);

				return
				{
					{
						QObject::tr ("Open"),
						QObject::tr ("Open the review page on ReviewBoard."),
						QIcon::fromTheme ("document-open"),
						QColor { "red" },
						MessageListActionFlag::None,
						[url] (const MessageInfo&)
						{
							const auto& entity = Util::MakeEntity (QUrl { url }, {}, FromUserInitiated | OnlyHandle);
							Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (entity);
						},
						{}
					}
				};
			}
		};

		QUrl GetUnsubscribeUrl (const QString& text)
		{
			const auto& parts = text.split (',', Qt::SkipEmptyParts);

			QUrl email;
			QUrl url;
			for (auto part : parts)
			{
				part = part.simplified ();
				if (part.startsWith ('<'))
					part = part.mid (1, part.size () - 2);

				const auto& ascii = part.toLatin1 ();

				if (ascii.startsWith ("mailto:"))
				{
					const auto& testEmail = QUrl::fromEncoded (ascii);
					if (testEmail.isValid ())
						email = testEmail;
				}
				else
				{
					const auto& testUrl = QUrl::fromEncoded (ascii);
					if (testUrl.isValid ())
						url = testUrl;
				}
			}

			return email.isValid () ? email : url;
		}

		QString GetListName (const MessageInfo& info, const Header_ptr& headers)
		{
			const auto& addrString = "<em>" +
					GetNiceMail (info.Addresses_ [AddressType::From].value (0)).toHtmlEscaped () +
					"</em>";

			const auto header = headers->findField ("List-Id");
			if (!header)
				return addrString;

			const auto& vmimeText = header->getValue<vmime::text> ();
			if (!vmimeText)
				return addrString;

			return "<em>" + StringizeCT (*vmimeText).toHtmlEscaped () + "</em>";
		}

		void HandleUnsubscribeText (const QString& text, const MessageInfo& info, const Header_ptr& headers, Account *acc)
		{
			const auto& url = GetUnsubscribeUrl (text);

			const auto& addrString = GetListName (info, headers);
			if (url.scheme () == "mailto")
			{
				if (QMessageBox::question (nullptr,
						QObject::tr ("Unsubscription confirmation"),
						QObject::tr ("Are you sure you want to unsubscribe from %1? "
							"This will send an email to %2.")
							.arg (addrString)
							.arg ("<em>" + url.path ().toHtmlEscaped () + "</em>"),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
					return;

				OutgoingMessage msg;
				msg.From_.Email_ = acc->GetConfig ().UserEmail_;
				msg.To_ = Addresses_t { { {}, url.path () } };
				const auto& subjQuery = Util::UrlAccessor { url } ["subject"];
				msg.Subject_ = subjQuery.isEmpty () ? "unsubscribe" : subjQuery;

				Util::Sequence (nullptr, acc->SendMessage (msg)) >>
						[url] (const auto& result)
						{
							const auto& entity = Util::Visit (result.AsVariant (),
									[url] (Util::Void)
									{
										return Util::MakeNotification ("Snails",
												QObject::tr ("Successfully sent unsubscribe request to %1.")
													.arg ("<em>" + url.path () + "</em>"),
												Priority::Info);
									},
									[url] (const auto& err)
									{
										const auto& msg = Util::Visit (err,
												[] (const auto& err) { return QString::fromUtf8 (err.what ()); });
										return Util::MakeNotification ("Snails",
												QObject::tr ("Unable to send unsubscribe request to %1: %2.")
													.arg ("<em>" + url.path () + "</em>")
													.arg (msg),
												Priority::Warning);
									});
							Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (entity);
						};
			}
			else
			{
				if (QMessageBox::question (nullptr,
						QObject::tr ("Unsubscription confirmation"),
						QObject::tr ("Are you sure you want to unsubscribe from %1? "
							"This will open the following web page in your browser: %2")
							.arg (addrString)
							.arg ("<br/><em>" + url.toString () + "</em>"),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
					return;

				const auto& entity = Util::MakeEntity (url, {}, FromUserInitiated | OnlyHandle);
				Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (entity);
			}
		}

		class UnsubscribeProvider final : public MessageListActionsProvider
		{
		public:
			QList<MessageListActionInfo> GetMessageActions (const MessageInfo&, const Header_ptr& headers, Account *acc) const override
			{
				const auto unsub = headers->findField ("List-Unsubscribe");
				if (!unsub)
					return {};

				return
				{
					{
						QObject::tr ("Unsubscribe"),
						QObject::tr ("Try unsubscribing from this maillist."),
						QIcon::fromTheme ("news-unsubscribe"),
						QColor { "blue" },
						MessageListActionFlag::None,
						[acc] (const MessageInfo& info)
						{
							const auto raw = acc->GetDatabase ()->GetMessageHeader (info.MessageId_);
							if (!raw)
								return;

							const auto& headers = HeaderFromContents (*raw);

							const auto unsub = headers->findField ("List-Unsubscribe");
							if (!unsub)
								return;

							const auto& unsubText = unsub->getValue<vmime::text> ();
							if (!unsubText)
								return;

							HandleUnsubscribeText (StringizeCT (*unsubText), info, headers, acc);
						},
						{}
					}
				};
			}
		};

		class DeleteProvider final : public MessageListActionsProvider
		{
		public:
			QList<MessageListActionInfo> GetMessageActions (const MessageInfo&, const Header_ptr&, Account *acc) const override
			{
				return
				{
					{
						QObject::tr ("Delete"),
						QObject::tr ("Delete the message"),
						QIcon::fromTheme ("edit-delete"),
						QColor {},
						MessageListActionFlag::AlwaysPresent,
						[acc] (const MessageInfo& info) { acc->DeleteMessages ({ info.FolderId_ }, info.Folder_); },
						{}
					}
				};
			}
		};

		class AttachmentsProvider final : public MessageListActionsProvider
		{
		public:
			QList<MessageListActionInfo> GetMessageActions (const MessageInfo& info, const Header_ptr&, Account *acc) const override
			{
				if (info.Attachments_.isEmpty ())
					return {};

				return
				{
					{
						QObject::tr ("Attachments"),
						QObject::tr ("Open/save attachments."),
						QIcon::fromTheme ("mail-attachment"),
						QColor { "green" },
						MessageListActionFlag::None,
						[acc] (const MessageInfo& info)
						{
							const auto iem = Core::Instance ().GetProxy ()->GetEntityManager ();

							const auto menu = new QMenu;
							menu->setAttribute (Qt::WA_DeleteOnClose);
							for (const auto& att : info.Attachments_)
								menu->addAction (att.GetName (),
										[=]
										{
											RunAttachmentSaveDialog (acc, iem,
													info.FolderId_, info.Folder_, att.GetName ());
										});
							menu->popup (QCursor::pos ());
						},
						{}
					}
				};
			}
		};
	}

	MessageListActionsManager::MessageListActionsManager (Account *acc, QObject *parent)
	: QObject { parent }
	, Acc_ { acc }
	{
		Providers_ << std::make_shared<AttachmentsProvider> ();
		Providers_ << std::make_shared<GithubProvider> ();
		Providers_ << std::make_shared<RedmineProvider> ();
		Providers_ << std::make_shared<BugzillaProvider> ();
		Providers_ << std::make_shared<ReviewboardProvider> ();
		Providers_ << std::make_shared<UnsubscribeProvider> ();
		Providers_ << std::make_shared<DeleteProvider> ();
	}

	QList<MessageListActionInfo> MessageListActionsManager::GetMessageActions (const MessageInfo& info) const
	{
		const auto headerText = Acc_->GetDatabase ()->GetMessageHeader (info.MessageId_);
		if (!headerText)
			return {};

		auto header = HeaderFromContents (*headerText);

		QList<MessageListActionInfo> result;
		for (const auto& provider : Providers_)
			result += provider->GetMessageActions (info, header, Acc_);
		return result;
	}
}
}
