/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "commands.h"
#include <boost/range/adaptor/reversed.hpp>
#include <QStringList>
#include <QtDebug>
#include <QTimer>
#include <QImage>
#include <QWidget>
#include <util/util.h>
#include <util/sll/slotclosure.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupportnonroster.h>
#include <interfaces/azoth/imetainfoentry.h>
#include <interfaces/azoth/ihaveentitytime.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/imucjoinwidget.h>
#include <interfaces/azoth/ihavepings.h>
#include <interfaces/azoth/imucperms.h>
#include <interfaces/azoth/ihavequeriableversion.h>
#include <interfaces/azoth/isupportlastactivity.h>
#include <interfaces/azoth/ihaveservicediscovery.h>
#include <interfaces/azoth/iprovidecommands.h>
#include <interfaces/azoth/imucprotocol.h>

namespace LC
{
namespace Azoth
{
namespace MuCommands
{
	namespace
	{
		void InjectMessage (IProxyObject *azothProxy, ICLEntry *entry,
				const QString& rich)
		{
			const auto entryObj = entry->GetQObject ();
			const auto msgObj = azothProxy->CreateCoreMessage (rich,
					QDateTime::currentDateTime (),
					IMessage::Type::ServiceMessage,
					IMessage::Direction::In,
					entryObj,
					entryObj);
			const auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Store ();
		}
	}

	bool HandleNames (IProxyObject *azothProxy, ICLEntry *entry, const QString&)
	{
		const auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());

		QStringList names;
		for (const auto obj : mucEntry->GetParticipants ())
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< obj
						<< "doesn't implement ICLEntry";
				continue;
			}
			const QString& name = entry->GetEntryName ();
			if (!name.isEmpty ())
				names << name;
		}
		names.sort ();

		const auto& contents = QObject::tr ("MUC's participants: ") + "<ul><li>" +
				names.join ("</li><li>") + "</li></ul>";
		InjectMessage (azothProxy, entry, contents);

		return true;
	}

	namespace
	{
		QHash<QString, ICLEntry*> GetParticipants (IMUCEntry *entry)
		{
			if (!entry)
				return {};

			QHash<QString, ICLEntry*> result;
			for (const auto entryObj : entry->GetParticipants ())
			{
				const auto entry = qobject_cast<ICLEntry*> (entryObj);
				if (entry)
					result [entry->GetEntryName ()] = entry;
			}
			return result;
		}

		QStringList ParseNicks (ICLEntry *entry, const QString& text)
		{
			auto split = text
					.section (' ', 1)
					.split ('\n', Qt::SkipEmptyParts);

			if (!split.isEmpty ())
				return split;

			const auto& msgs = entry->GetAllMessages ();
			for (const auto msg : boost::adaptors::reverse (msgs))
			{
				if (const auto otherPart = qobject_cast<ICLEntry*> (msg->OtherPart ()))
				{
					split << otherPart->GetEntryName ();
					break;
				}
			}

			return split;
		}

		ICLEntry* ResolveEntry (const QString& name, const QHash<QString, ICLEntry*>& context, IAccount *acc, ICLEntry *hint)
		{
			if (context.contains (name))
				return context.value (name);

			QList<ICLEntry*> entries;
			for (const auto entryObj : acc->GetCLEntries ())
			{
				const auto entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry)
					continue;

				if (entry->GetEntryName () == name || entry->GetHumanReadableID () == name)
					entries << entry;
			}

			if (!entries.isEmpty ())
				return entries.contains (hint) ? hint : entries.first ();

			if (const auto isn = qobject_cast<ISupportNonRoster*> (acc->GetQObject ()))
				if (const auto entry = qobject_cast<ICLEntry*> (isn->CreateNonRosterItem (name)))
					return entry;

			return nullptr;
		}

		QString FormatRepresentation (const QList<QPair<QString, QVariant>>& repr)
		{
			QStringList strings;

			for (const auto& pair : repr)
			{
				if (pair.second.isNull ())
					continue;

				auto string = "<strong>" + pair.first + ":</strong> ";

				switch (pair.second.type ())
				{
				case QVariant::String:
				{
					const auto& metaStr = pair.second.toString ();
					if (metaStr.isEmpty ())
						continue;

					string += metaStr;
					break;
				}
				case QVariant::Image:
				{
					const auto& image = pair.second.value<QImage> ();
					if (image.isNull ())
						continue;

					const auto& src = Util::GetAsBase64Src (image);
					string += "<img src='" + src + "' alt=''/>";
					break;
				}
				case QVariant::Date:
				{
					const auto& date = pair.second.toDate ();
					if (date.isNull ())
						continue;

					string += QLocale {}.toString (date, QLocale::LongFormat);
					break;
				}
				case QVariant::StringList:
				{
					const auto& list = pair.second.toStringList ();
					if (list.isEmpty ())
						continue;

					string += "<ul><li>" + list.join ("</li><li>") + "</li></ul>";
					break;
				}
				default:
					string += "unhandled data type ";
					string += pair.second.typeName ();
					break;
				}

				strings << string;
			}

			if (strings.isEmpty ())
				return {};

			return "<ul><li>" + strings.join ("</li><li>") + "</li></ul>";
		}

		template<typename T, typename F>
		void PerformAction (T action, F fallback, ICLEntry *entry, const QString& text)
		{
			auto nicks = ParseNicks (entry, text);
			if (nicks.isEmpty ())
			{
				if (entry->GetEntryType () == ICLEntry::EntryType::MUC)
					return;
				else
					nicks << entry->GetHumanReadableID ();
			}

			const auto& participants = GetParticipants (qobject_cast<IMUCEntry*> (entry->GetQObject ()));
			for (const auto& name : nicks)
			{
				const auto target = ResolveEntry (name.trimmed (),
						participants, entry->GetParentAccount (), entry);
				if (!target)
				{
					fallback (name);
					continue;
				}

				action (target, name);
			}
		}

		template<typename T>
		void PerformAction (T action, IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
		{
			PerformAction (action,
					[azothProxy, entry] (const QString& name)
					{
						InjectMessage (azothProxy, entry,
								QObject::tr ("Unable to resolve %1.").arg ("<em>" + name + "</em>"));
					},
					entry, text);
		}
	}

	bool ShowVCard (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		PerformAction ([azothProxy, entry, text] (ICLEntry *target, const QString& name) -> void
				{
					const auto targetObj = target->GetQObject ();
					const auto imie = qobject_cast<IMetaInfoEntry*> (targetObj);
					if (!imie)
					{
						InjectMessage (azothProxy, entry,
								QObject::tr ("%1 doesn't support extended metainformation.").arg ("<em>" + name + "</em>"));
						return;
					}

					const auto& repr = FormatRepresentation (imie->GetVCardRepresentation ());
					if (repr.isEmpty ())
					{
						InjectMessage (azothProxy, entry,
								name + ": " + QObject::tr ("no information, would wait for next vcard update..."));

						new Util::SlotClosure<Util::DeleteLaterPolicy>
						{
							[azothProxy, entry, text] { ShowVCard (azothProxy, entry, text); },
							targetObj,
							SIGNAL (vcardUpdated ()),
							targetObj
						};
					}
					else
						InjectMessage (azothProxy, entry, name + ":<br/>" + repr);
				},
				azothProxy, entry, text);

		return true;
	}

	namespace
	{
		void ShowVersionVariant (IProxyObject *azothProxy, ICLEntry *entry,
				const QString& name, ICLEntry *target, const QString& var, bool initial)
		{
			const auto ihqv = qobject_cast<IHaveQueriableVersion*> (target->GetQObject ());
			const auto& info = target->GetClientInfo (var);

			QStringList fields;
			auto add = [&fields] (const QString& name, const QString& value)
			{
				if (!value.isEmpty ())
					fields << "<strong>" + name + ":</strong> " + value;
			};

			add (QObject::tr ("Type"), info ["client_type"].toString ());
			add (QObject::tr ("Name"), info ["client_name"].toString ());
			add (QObject::tr ("Version"), info ["client_version"].toString ());
			add (QObject::tr ("OS"), info ["client_os"].toString ());

			if (initial && !info.contains ("client_version") && ihqv)
			{
				const auto pendingObj = ihqv->QueryVersion (var);

				const auto closure = new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[name, azothProxy, entry, target, var]
					{
						ShowVersionVariant (azothProxy, entry, name, target, var, false);
					},
					pendingObj,
					SIGNAL (versionReceived ()),
					pendingObj
				};
				QTimer::singleShot (10 * 1000, closure, SLOT (run ()));
				return;
			}

			auto body = QObject::tr ("Client information for %1:")
					.arg (var.isEmpty () && target->Variants ().size () == 1 ?
							name :
							target->GetHumanReadableID () + '/' + var);
			body += fields.isEmpty () ?
					QObject::tr ("no information available.") :
					"<ul><li>" + fields.join ("</li><li>") + "</li></ul>";

			InjectMessage (azothProxy, entry, body);
		}
	}

	bool ShowVersion (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		PerformAction ([azothProxy, entry] (ICLEntry *target, const QString& name)
				{
					for (const auto& var : target->Variants ())
						ShowVersionVariant (azothProxy, entry, name, target, var, true);
				},
				azothProxy, entry, text);

		return true;
	}

	namespace
	{
		QString FormatTzo (int tzo)
		{
			const auto& time = QTime { 0, 0 }.addSecs (std::abs (tzo));

			auto str = time.toString ("HH:mm");
			str.prepend (tzo >= 0 ? '+' : '-');
			return str;
		}
	}

	bool ShowTime (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		PerformAction ([azothProxy, entry, text] (ICLEntry *target, const QString& name) -> void
				{
					const auto targetObj = target->GetQObject ();
					const auto ihet = qobject_cast<IHaveEntityTime*> (targetObj);
					if (!ihet)
					{
						InjectMessage (azothProxy, entry,
								QObject::tr ("%1 does not support querying time.")
										.arg (name));
						return;
					}

					bool shouldUpdate = false;

					QStringList fields;

					const auto& variants = target->Variants ();
					for (const auto& var : variants)
					{
						const auto time = target->GetClientInfo (var)
								.value ("client_time").toDateTime ();
						const auto& varName = var.isEmpty () ?
								name :
								target->GetHumanReadableID () + '/' + var;
						if (!time.isValid ())
						{
							shouldUpdate = true;
							continue;
						}

						const auto tzo = target->GetClientInfo (var)
								.value ("client_tzo").toInt ();

						QString field = QObject::tr ("Current time for %1:")
								.arg (varName);
						field += "<ul><li>";
						field += QObject::tr ("Local time: %1")
								.arg (azothProxy->PrettyPrintDateTime (time));
						field += "</li><li>";
						field += QObject::tr ("Timezone: %1")
								.arg (FormatTzo (tzo));
						field += "</li><li>";

						const auto& utcTime = time.addSecs (-tzo);
						field += QObject::tr ("UTC time: %1")
								.arg (azothProxy->PrettyPrintDateTime (utcTime));

						field += "</li></ul>";
						fields << field;
					}

					if (shouldUpdate)
					{
						ihet->UpdateEntityTime ();

						new Util::SlotClosure<Util::DeleteLaterPolicy>
						{
							[azothProxy, entry, text] { ShowTime (azothProxy, entry, text); },
							targetObj,
							SIGNAL (entityTimeUpdated ()),
							targetObj
						};
					}

					if (fields.isEmpty ())
						return;

					const auto& body = "<ul><li>" + fields.join ("</li><li>") + "</li></ul>";
					InjectMessage (azothProxy, entry,
							QObject::tr ("Entity time for %1:").arg (name) + body);
				},
				azothProxy, entry, text);

		return true;
	}

	bool Disco (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		const auto accObj = entry->GetParentAccount ()->GetQObject ();
		const auto ihsd = qobject_cast<IHaveServiceDiscovery*> (accObj);
		if (!ihsd)
		{
			InjectMessage (azothProxy, entry,
					QObject::tr ("%1's account does not support service discovery.")
							.arg (entry->GetEntryName ()));
			return true;
		}

		const auto requestSD = [ihsd, azothProxy, entry, accObj] (const QString& query)
		{
			const auto sessionObj = ihsd->CreateSDSession ();
			const auto session = qobject_cast<ISDSession*> (sessionObj);
			if (!session)
			{
				InjectMessage (azothProxy, entry,
						QObject::tr ("Unable to create service discovery session for %1.")
								.arg ("<em>" + query + "</em>"));
				return;
			}

			session->SetQuery (query);

			QMetaObject::invokeMethod (accObj,
					"gotSDSession",
					Q_ARG (QObject*, sessionObj));
		};

		PerformAction ([requestSD] (ICLEntry *target, const QString&) { requestSD (target->GetHumanReadableID ()); },
				[requestSD] (const QString& name) { requestSD (name); },
				entry, text);

		return true;
	}

	bool JoinMuc (IProxyObject*, ICLEntry *entry, const QString& text)
	{
		const auto acc = entry->GetParentAccount ();
		const auto mucProto = qobject_cast<IMUCProtocol*> (acc->GetParentProtocol ());
		if (!mucProto)
			throw CommandException { QObject::tr ("The account %1 does not support MUCs.")
					.arg (acc->GetAccountName ()) };

		const auto data = mucProto->TryGuessMUCIdentifyingData (text.section (' ', 1), entry->GetQObject ());
		if (data.isEmpty ())
			throw CommandException { QObject::tr ("Cannot guess MUC connection parameters.") };

		std::unique_ptr<QWidget> jw { mucProto->GetMUCJoinWidget () };
		if (!jw)
			throw CommandException { QObject::tr ("Cannot join the MUC.") };

		const auto imjw = qobject_cast<IMUCJoinWidget*> (jw.get ());
		imjw->SetIdentifyingData (data);
		imjw->Join (acc->GetQObject ());

		return true;
	}

	bool RejoinMuc (IProxyObject*, ICLEntry *entry, const QString& text)
	{
		const auto acc = entry->GetParentAccount ();
		const auto entryObj = entry->GetQObject ();
		const auto mucEntry = qobject_cast<IMUCEntry*> (entryObj);
		if (!mucEntry)
			return false;

		const auto& mucData = mucEntry->GetIdentifyingData ();

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[entryObj, acc, mucData] () -> void
			{
				if (acc->GetCLEntries ().contains (entryObj))
					return;

				QTimer::singleShot (1000,
						[acc, mucData]
						{
							const auto proto = qobject_cast<IMUCProtocol*> (acc->GetParentProtocol ());
							if (!proto)
								return;

							std::unique_ptr<QWidget> jw { proto->GetMUCJoinWidget () };
							if (!jw)
								return;

							const auto imjw = qobject_cast<IMUCJoinWidget*> (jw.get ());
							imjw->SetIdentifyingData (mucData);
							imjw->Join (acc->GetQObject ());
						});
			},
			acc->GetQObject (),
			SIGNAL (removedCLItems (QList<QObject*>)),
			entryObj
		};

		mucEntry->Leave (text.section (' ', 1));

		return true;
	}

	bool LeaveMuc (IProxyObject*, ICLEntry *entry, const QString& text)
	{
		const auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());
		if (!mucEntry)
			return false;

		mucEntry->Leave (text.section (' ', 1));
		return true;
	}

	bool ChangeSubject (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		const auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());
		if (!mucEntry)
			return false;

		const auto& newSubject = text.section (' ', 1);
		if (newSubject.trimmed ().isEmpty ())
			InjectMessage (azothProxy, entry, mucEntry->GetMUCSubject ());
		else
			mucEntry->SetMUCSubject (newSubject);
		return true;
	}

	bool ChangeNick (IProxyObject*, ICLEntry *entry, const QString& text)
	{
		const auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());
		if (!mucEntry)
			return false;

		const auto& newNick = text.section (' ', 1);
		if (newNick.isEmpty ())
			return false;

		mucEntry->SetNick (newNick);
		return true;
	}

	namespace
	{
		void PerformRoleAction (const QPair<QByteArray, QByteArray>& role,
				QObject *mucEntryObj, QString str)
		{
			if (role.first.isEmpty () && role.second.isEmpty ())
				return;

			str = str.trimmed ();
			const int pos = str.lastIndexOf ('|');
			const auto& nick = pos > 0 ? str.left (pos) : str;
			const auto& reason = pos > 0 ? str.mid (pos + 1) : QString ();

			auto mucEntry = qobject_cast<IMUCEntry*> (mucEntryObj);
			auto mucPerms = qobject_cast<IMUCPerms*> (mucEntryObj);

			const auto& parts = mucEntry->GetParticipants ();
			auto partPos = std::find_if (parts.begin (), parts.end (),
					[&nick] (QObject *entryObj) -> bool
					{
						auto entry = qobject_cast<ICLEntry*> (entryObj);
						return entry && entry->GetEntryName () == nick;
					});
			if (partPos == parts.end ())
				return;

			mucPerms->SetPerm (*partPos, role.first, role.second, reason);
		}
	}

	bool Kick (IProxyObject*, ICLEntry *entry, const QString& text)
	{
		const auto mucPerms = qobject_cast<IMUCPerms*> (entry->GetQObject ());
		if (!mucPerms)
			return false;

		PerformRoleAction (mucPerms->GetKickPerm (), entry->GetQObject (), text.section (' ', 1));
		return true;
	}

	bool Ban (IProxyObject*, ICLEntry *entry, const QString& text)
	{
		const auto mucPerms = qobject_cast<IMUCPerms*> (entry->GetQObject ());
		if (!mucPerms)
			return false;

		PerformRoleAction (mucPerms->GetBanPerm (), entry->GetQObject (), text.section (' ', 1));
		return true;
	}

	namespace
	{
		ICLEntry* GetMucEntry (ICLEntry *entry)
		{
			switch (entry->GetEntryType ())
			{
			case ICLEntry::EntryType::MUC:
				return entry;
			case ICLEntry::EntryType::PrivateChat:
				return entry->GetParentCLEntry ();
			default:
				return nullptr;
			}
		}
	}

	bool ListPerms (IProxyObject *azothProxy, ICLEntry *entry, const QString&)
	{
		const auto mucEntry = GetMucEntry (entry);
		if (!mucEntry)
		{
			InjectMessage (azothProxy, entry,
					QObject::tr ("%1 is not related to a multiuser chat room.")
							.arg ("<em>" + entry->GetEntryName () + "</em>"));
			return true;
		}

		const auto mucPerms = qobject_cast<IMUCPerms*> (mucEntry->GetQObject ());
		if (!mucPerms)
		{
			const auto acc = entry->GetParentAccount ();
			const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
			InjectMessage (azothProxy, entry,
					QObject::tr ("%1 (or its protocol %2) does not support permissions.")
							.arg ("<em>" + entry->GetEntryName () + "</em>")
							.arg (proto->GetProtocolName ()));
			return true;
		}

		QStringList classes;
		for (const auto& classInfo : Util::Stlize (mucPerms->GetPossiblePerms ()))
		{
			const auto& classId = classInfo.first;

			QStringList fields;
			for (const auto& role : classInfo.second)
				fields << QString ("%1 (%2)")
						.arg ("<code>" + QString::fromUtf8 (role) + "</code>")
						.arg (mucPerms->GetUserString (role));

			auto string = QObject::tr ("Permission class %1 (%2):")
					.arg ("<code>" + QString::fromUtf8 (classId) + "</code>")
					.arg (mucPerms->GetUserString (classId));
			string += "<ul><li>" + fields.join ("</li><li>") + "</li></ul>";

			classes << string;
		}

		InjectMessage (azothProxy, entry,
				QObject::tr ("Available role classes and their values:") +
					"<ul><li>" + classes.join ("</li><li>") + "</li></ul>");

		return true;
	}

	namespace
	{
		class PermSetter
		{
			IProxyObject * const AzothProxy_;
			ICLEntry * const Entry_;
			ICLEntry * const MucEntry_;
			IMUCPerms * const MucPerms_;

			const QString Text_;
			const QString Command_;
			const QString PermClassStr_;
			QByteArray PermClass_;
			const QString PermValueStr_;
			QByteArray PermValue_;

			const QString Remainder_;
			const QString Nick_;
			const QString Reason_;

			enum class Mode
			{
				Nick,
				Id
			} Mode_ = Mode::Nick;
		public:
			PermSetter (IProxyObject *azothProxy, ICLEntry *entry, const QString& text);
		private:
			bool CheckPermsCast () const;
			bool CheckSyntax () const;
			bool CheckMode ();
			bool CheckParticipants (decltype (GetParticipants ({}))) const;
			bool CheckPermsValidity ();

			void SetNickPerms (decltype (GetParticipants ({})));
			void SetIdPerms ();
		};

		PermSetter::PermSetter (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
		: AzothProxy_ { azothProxy }
		, Entry_ { entry }
		, MucEntry_ { GetMucEntry (entry) }
		, MucPerms_ { MucEntry_ ? qobject_cast<IMUCPerms*> (MucEntry_->GetQObject ()) : nullptr }
		, Text_ { text }
		, Command_ { text.section (' ', 0, 0) }
		, PermClassStr_ { text.section (' ', 1, 1) }
		, PermClass_ { PermClassStr_.toUtf8 () }
		, PermValueStr_ { text.section (' ', 2, 2) }
		, PermValue_ { PermValueStr_.toUtf8 () }
		, Remainder_ { text.section (' ', 4) }
		, Nick_ { Remainder_.section ('\n', 0, 0) }
		, Reason_ { Remainder_.section ('\n', 1) }
		{
			if (!CheckPermsCast () || !CheckSyntax () || !CheckMode () || !CheckPermsValidity ())
				return;

			switch (Mode_)
			{
			case Mode::Nick:
			{
				const auto& parts = GetParticipants (qobject_cast<IMUCEntry*> (MucEntry_->GetQObject ()));
				if (!CheckParticipants (parts))
					return;

				SetNickPerms (parts);
				break;
			}
			case Mode::Id:
				SetIdPerms ();
				break;
			}
		}

		bool PermSetter::CheckPermsCast () const
		{
			if (!MucPerms_)
			{
				const auto acc = Entry_->GetParentAccount ();
				const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("%1 (or its protocol %2) does not support permissions.")
								.arg ("<em>" + Entry_->GetEntryName () + "</em>")
								.arg (proto->GetProtocolName ()));

				return false;
			}

			return true;
		}

		bool PermSetter::CheckSyntax () const
		{
			if (PermClassStr_.isEmpty () ||
					PermValueStr_.isEmpty () ||
					Nick_.isEmpty ())
			{
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("Invalid syntax. Type %1 for more information.")
								.arg ("<code>/help " + Command_ + "</code>"));
				return false;
			}

			return true;
		}

		bool PermSetter::CheckMode ()
		{
			const auto& modeStr = Text_.section (' ', 3, 3);
			if (modeStr == "nick")
				Mode_ = Mode::Nick;
			else if (modeStr == "id")
				Mode_ = Mode::Id;
			else
			{
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("Unknown mode %1. Type %2 for more information.")
								.arg ("<em>" + modeStr + "</em>")
								.arg ("<code>/help " + Command_ + "</code>"));
				return false;
			}

			return true;
		}

		bool PermSetter::CheckParticipants (decltype (GetParticipants ({})) parts) const
		{
			if (!parts.contains (Nick_))
			{
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("Unknown participant %1.")
								.arg ("<em>" + Nick_ + "</em>"));
				return false;
			}

			return true;
		}

		namespace
		{
			int GetExpandCount (const QList<QByteArray>& variants, const QByteArray& given)
			{
				return std::count_if (variants.begin (), variants.end (),
						[&given] (const QByteArray& variant) { return variant.startsWith (given); });
			}

			bool Expand (const QList<QByteArray>& variants, QByteArray& given)
			{
				const auto pos = std::find_if (variants.begin (), variants.end (),
						[&given] (const QByteArray& variant) { return variant.startsWith (given); });
				if (*pos == given)
					return false;

				given = *pos;
				return true;
			}
		}

		bool PermSetter::CheckPermsValidity ()
		{
			const auto& perms = MucPerms_->GetPossiblePerms ();

			const auto& keys = perms.keys ();
			if (GetExpandCount (keys, PermClass_) != 1)
			{
				const auto& keys = Util::Map (perms.keys (),
						[] (const QByteArray& ba) { return QString::fromUtf8 (ba); });
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("Unknown or ambiguous permission class %1, available classes are: %2")
								.arg ("<code>" + PermClassStr_ + "</code>")
								.arg ("<ul><li>" + QStringList { keys }.join ("</li><li>") + "</ul></li>"));
				return false;
			}

			if (Expand (keys, PermClass_))
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("Expanded requested permission class to %1 (%2)")
								.arg ("<code>" + QString::fromUtf8 (PermClass_) + "</code>")
								.arg (MucPerms_->GetUserString (PermClass_)));

			const auto& values = perms [PermClass_];
			if (GetExpandCount (values, PermValue_) != 1)
			{
				const auto& valuesStrs = Util::Map (values,
						[] (const QByteArray& ba) { return QString::fromUtf8 (ba); });
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("Unknown or ambiguous permission class %1, available classes are: %2")
								.arg ("<code>" + PermValueStr_ + "</code>")
								.arg ("<ul><li>" + QStringList { valuesStrs }.join ("</li><li>") + "</ul></li>"));
				return false;
			}

			if (Expand (values, PermValue_))
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("Expanded requested permission value to %1 (%2)")
								.arg ("<code>" + QString::fromUtf8 (PermValue_) + "</code>")
								.arg (MucPerms_->GetUserString (PermValue_)));

			return true;
		}

		void PermSetter::SetNickPerms (decltype (GetParticipants ({})) parts)
		{
			const auto part = parts [Nick_];
			const auto partObj = part->GetQObject ();

			if (!MucPerms_->MayChangePerm (partObj, PermClass_, PermValue_))
			{
				InjectMessage (AzothProxy_, Entry_,
						QObject::tr ("Cannot change %1's role of class %2 (%3) to %4 (%5).")
								.arg ("<em>" + Nick_ + "</em>")
								.arg ("<code>" + PermClassStr_ + "</code>")
								.arg ("<em>" + MucPerms_->GetUserString (PermClass_) + "</em>")
								.arg ("<code>" + PermValueStr_ + "</code>")
								.arg ("<em>" + MucPerms_->GetUserString (PermValue_) + "</em>"));
				return;
			}

			MucPerms_->SetPerm (partObj, PermClass_, PermValue_, Reason_);
		}

		void PermSetter::SetIdPerms ()
		{
			MucPerms_->TrySetPerm (Nick_, PermClass_, PermValue_, Reason_);
		}
	}

	bool SetPerm (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		PermSetter { azothProxy, entry, text };
		return true;
	}

	namespace
	{
		QString GetLastActivityPattern (IPendingLastActivityRequest::Context context)
		{
			switch (context)
			{
			case IPendingLastActivityRequest::Context::Activity:
				return QObject::tr ("Last activity of %1: %2.");
			case IPendingLastActivityRequest::Context::LastConnection:
				return QObject::tr ("Last connection of %1: %2.");
			case IPendingLastActivityRequest::Context::Uptime:
				return QObject::tr ("%1's uptime: %2.");
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown context"
					<< static_cast<int> (context);
			return {};
		}
	}

	bool Last (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		const auto handlePending = [azothProxy, entry] (QObject *pending, const QString& name) -> void
		{
			if (!pending)
			{
				InjectMessage (azothProxy, entry,
						QObject::tr ("%1 does not support last activity.").arg (name));
				return;
			}

			new Util::SlotClosure<Util::DeleteLaterPolicy>
			{
				[pending, azothProxy, entry, name] ()
				{
					const auto iplar = qobject_cast<IPendingLastActivityRequest*> (pending);
					const auto& time = Util::MakeTimeFromLong (iplar->GetTime ());
					const auto& pattern = GetLastActivityPattern (iplar->GetContext ());
					InjectMessage (azothProxy, entry, pattern.arg (name).arg (time));
				},
				pending,
				SIGNAL (gotLastActivity ()),
				pending
			};
		};

		PerformAction ([handlePending] (ICLEntry *target, const QString& name) -> void
				{
					const auto isla = qobject_cast<ISupportLastActivity*> (target->
								GetParentAccount ()->GetQObject ());
					const auto pending = isla ?
							isla->RequestLastActivity (target->GetQObject (), {}) :
							nullptr;
					handlePending (pending, name);
				},
				[entry, handlePending] (const QString& name) -> void
				{
					const auto isla = qobject_cast<ISupportLastActivity*> (entry->
								GetParentAccount ()->GetQObject ());
					const auto pending = isla ?
							isla->RequestLastActivity (name) :
							nullptr;
					handlePending (pending, name);
				},
				entry, text);

		return true;
	}

	bool Invite (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		const auto& id = text.section (' ', 1, 1);
		const auto& reason = text.section (' ', 2);

		if (entry->GetEntryType () == ICLEntry::EntryType::MUC)
		{
			const auto invitee = ResolveEntry (id, {}, entry->GetParentAccount (), entry);
			const auto& inviteeId = invitee ?
					invitee->GetHumanReadableID () :
					id;

			const auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());
			mucEntry->InviteToMUC (inviteeId, reason);

			InjectMessage (azothProxy, entry, QObject::tr ("Invited %1 to %2.")
					.arg (inviteeId)
					.arg (entry->GetEntryName ()));
		}
		else
		{
			const auto mucEntry = ResolveEntry (id, {}, entry->GetParentAccount (), entry);
			if (!mucEntry)
			{
				InjectMessage (azothProxy, entry,
						QObject::tr ("Unable to resolve multiuser chat for %1.").arg (id));
				return true;
			}

			const auto mucIface = qobject_cast<IMUCEntry*> (mucEntry->GetQObject ());
			if (!mucIface)
			{
				InjectMessage (azothProxy, entry,
						QObject::tr ("%1 is not a multiuser chat.").arg (id));
				return true;
			}

			mucIface->InviteToMUC (entry->GetHumanReadableID (), reason);
			InjectMessage (azothProxy, entry, QObject::tr ("Invited %1 to %2.")
					.arg (entry->GetEntryName ())
					.arg (mucEntry->GetEntryName ()));
		}

		return true;
	}

	namespace
	{
		void WhoisImpl (IProxyObject *azothProxy, ICLEntry *entry, ICLEntry *partEntry, ICLEntry *showEntry, const QString& text)
		{
			const auto& reqNick = text.section (' ', 1);

			const auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());
			const auto part = reqNick.isEmpty () ?
					partEntry :
					GetParticipants (mucEntry).value (reqNick);

			if (!part)
			{
				InjectMessage (azothProxy, showEntry,
						QObject::tr ("Unable to find participant %1.")
								.arg ("<em>" + reqNick + "</em>"));
				return;
			}

			const auto& nick = part->GetEntryName ();

			const auto& rid = mucEntry->GetRealID (part->GetQObject ());
			if (rid.isEmpty ())
				InjectMessage (azothProxy, showEntry,
						QObject::tr ("Unable to get real ID of %1.")
								.arg ("<em>" + nick + "</em>"));
			else
				InjectMessage (azothProxy, showEntry,
						QObject::tr ("%1's real ID: %2.")
								.arg ("<em>" + nick + "</em>")
								.arg ("<em>" + rid + "</em>"));
		}
	}

	bool Whois (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		switch (entry->GetEntryType ())
		{
		case ICLEntry::EntryType::MUC:
			WhoisImpl (azothProxy, entry, nullptr, entry, text);
			break;
		case ICLEntry::EntryType::PrivateChat:
			WhoisImpl (azothProxy,
					entry->GetParentCLEntry (),
					entry,
					entry,
					text);
			break;
		default:
			break;
		}
		return true;
	}

	bool Pm (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		if (entry->GetEntryType () != ICLEntry::EntryType::MUC)
			return false;

		const auto& firstLine = text.section ('\n', 0, 0);
		const auto& message = text.section ('\n', 1);
		const auto& nick = firstLine.section (' ', 1);

		const auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());
		const auto part = GetParticipants (mucEntry).value (nick);
		if (!part)
		{
			InjectMessage (azothProxy, entry,
					QObject::tr ("Unable to find participant %1.")
							.arg ("<em>" + nick + "</em>"));
			return true;
		}

		const auto msg = part->CreateMessage (IMessage::Type::ChatMessage,
				part->Variants ().value (0), message);
		msg->Send ();
		return true;
	}

	bool Ping (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		PerformAction ([azothProxy, entry] (ICLEntry *target, const QString& name) -> void
				{
					const auto targetObj = target->GetQObject ();
					const auto ihp = qobject_cast<IHavePings*> (targetObj);
					if (!ihp)
					{
						InjectMessage (azothProxy, entry,
								QObject::tr ("%1 does not support pinging.").arg (name));
						return;
					}

					const auto reply = ihp->Ping ({});
					new Util::SlotClosure<Util::DeleteLaterPolicy>
					{
						[reply, azothProxy, entry, name] ()
						{
							const auto ipp = qobject_cast<IPendingPing*> (reply);

							InjectMessage (azothProxy, entry,
									QObject::tr ("Pong from %1: %2 ms.")
											.arg (name)
											.arg (ipp->GetTimeout ()));
						},
						reply,
						SIGNAL (replyReceived (int)),
						reply
					};
				},
				azothProxy, entry, text);

		return true;
	}

	TextMorphResult Subst (IProxyObject*, ICLEntry*, const QString& text)
	{
		const char sep = text.count ('\n') < 2 ?
				' ' :
				'\n';
		const auto& pattern = text.section (sep, 1, 1);
		const auto& replacement = text.section (sep, 2, 2);
		return { text.section (sep, 3).replace (pattern, replacement) };
	}
}
}
}
