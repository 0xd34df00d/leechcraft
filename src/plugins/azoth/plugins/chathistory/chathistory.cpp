/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "chathistory.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QIcon>
#include <plugininterface/util.h>
#include <plugininterface/dblock.h>
#include <interfaces/imessage.h>
#include <interfaces/iclentry.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace ChatHistory
				{
					void Plugin::Init (ICoreProxy_ptr proxy)
					{
					}

					void Plugin::SecondInit ()
					{
						DB_.reset (new QSqlDatabase (QSqlDatabase::addDatabase ("QSQLITE", "History connection")));
						Util::CreateIfNotExists ("azoth");
						// TODO get rid of the hardcoded path below somehow
						DB_->setDatabaseName (QDir::homePath () + "/.leechcraft/azoth/history.sqlite");
						DB_->open();
						if (!DB_->tables ().contains ("history"))
							InitializeTables ();
						UserSelecter_ = QSqlQuery (*DB_);
						UserSelecter_.prepare ("SELECT EntryID FROM azoth_users ");
						UserIdSelecter_ = QSqlQuery (*DB_);
						UserIdSelecter_.prepare ("SELECT Id FROM azoth_users WHERE EntryID = :entry_id");
						UserInserter_ = QSqlQuery (*DB_);
						UserInserter_.prepare ("INSERT INTO azoth_users (EntryID) VALUES (:entry_id);");
						MessageDumper_ = QSqlQuery (*DB_);
						MessageDumper_.prepare ("INSERT INTO azoth_history (Id, Date, Direction, Message, Variant, Type) "
								"VALUES (:id, :date, :direction, :message, :variant, :type);");
						if (!UserSelecter_.exec ())
							LeechCraft::Util::DBLock::DumpError (UserSelecter_);
						else
						{
							while (UserSelecter_.next ()){
								QString entryId = UserSelecter_.value (0).toString ();
								Users_.insert (entryId);
							}
						}
					}

					QByteArray Plugin::GetUniqueID () const
					{
						return "org.LeechCraft.Azoth.ChatHistory";
					}

					void Plugin::Release ()
					{
						QSqlDatabase::database ("History connection").close ();
						DB_->close ();
						QSqlDatabase::removeDatabase ("History connection");
					}

					QString Plugin::GetName () const
					{
						return "Azoth ChatHistory";
					}

					QString Plugin::GetInfo () const
					{
						return tr ("Stores message history in Azoth.");
					}

					QIcon Plugin::GetIcon () const
					{
						return QIcon ();
					}

					QStringList Plugin::Provides () const
					{
						return QStringList ();
					}

					QStringList Plugin::Needs () const
					{
						return QStringList ();
					}

					QStringList Plugin::Uses () const
					{
						return QStringList ();
					}

					void Plugin::SetProvider (QObject*, const QString&)
					{
					}

					QSet<QByteArray> Plugin::GetPluginClasses () const
					{
						QSet<QByteArray> result;
						result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
						return result;
					}

					void Plugin::hookMessageCreated (IHookProxy_ptr proxy,
							QObject *chatTab, QObject *message)
					{
						qDebug () << Q_FUNC_INFO;
						IMessage *msg = qobject_cast<IMessage*> (message);
						if (!msg)
						{
							qWarning () << Q_FUNC_INFO
									<< message
									<< "doesn't implement IMessage"
									<< sender ();
							return;
						}
						ProcessMessageDump (msg);
					}
					void Plugin::hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
							 QObject *message)
					{
						qDebug () << Q_FUNC_INFO;
						IMessage *msg = qobject_cast<IMessage*> (message);
						if (!msg)
						{
							qWarning () << Q_FUNC_INFO
									<< message
									<< "doesn't implement IMessage"
									<< sender ();
							return;
						}
						ProcessMessageDump (msg);
					}

					void Plugin::ProcessMessageDump (IMessage *msg)
					{
						IMessage::Direction direction = msg->GetDirection ();
						IMessage::MessageType messageType = msg->GetMessageType ();
						QObject *otherPart = msg->OtherPart ();
						ICLEntry *entry = qobject_cast<ICLEntry*> (otherPart);
						if (!entry)
						{
							qWarning () << Q_FUNC_INFO
									<< "message's other part doesn't implement ICLEntry"
									<< msg->GetObject ()
									<< msg->OtherPart ();
							return;
						}
						QString entryName = entry->GetEntryName ();
						QString otherVariant = msg->GetOtherVariant ();
						QString body = msg->GetBody ();
						QDateTime timestamp = msg->GetDateTime ();
						if (!Users_.contains (entryName))
						{
							UserInserter_.bindValue (":entry_id", entryName);
							if (!UserInserter_.exec ())
							{
								LeechCraft::Util::DBLock::DumpError (UserInserter_);
								return;
							}
							UserInserter_.finish ();
						}
						else
						{
							UserIdSelecter_.bindValue (":entry_id", entryName);
							if (!UserIdSelecter_.exec ())
							{
								LeechCraft::Util::DBLock::DumpError (UserIdSelecter_);
								return;
							}
							else
							{
								UserIdSelecter_.first ();
								int id = UserIdSelecter_.value (0).toInt ();
								MessageDumper_.bindValue (":id", id);
								MessageDumper_.bindValue (":date", timestamp);
								MessageDumper_.bindValue (":direction", direction);
								MessageDumper_.bindValue (":message", body);
								MessageDumper_.bindValue (":variant", otherVariant);
								MessageDumper_.bindValue (":type", messageType);
								if (!MessageDumper_.exec ())
								{
									LeechCraft::Util::DBLock::DumpError (MessageDumper_);
									return;
								}
							}
							MessageDumper_.finish ();
						}
					}
					void Plugin::InitializeTables ()
					{
						//TODO Check queries for error
						QSqlQuery query (*DB_);
						query.exec ("CREATE TABLE azoth_users ("
								"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
								"EntryID TEXT "
								");");
						query.exec ("CREATE UNIQUE INDEX azoth_users_id "
								"ON azoth_users (Id);");
						query.exec ("CREATE TABLE azoth_history ("
								"Id INTEGER, "
								"Date DATETIME, "
								"Direction INTEGER, "
								"Message TEXT, "
								"OtherPart TEXT, "
								"Variant TEXT, "
								"Type INTEGER "
								");");
					}
				}
			}
		}
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_chathistory, LeechCraft::Plugins::Azoth::Plugins::ChatHistory::Plugin);

