/* Copyright (C) 2006 - 2011 Jan Kundrát <jkt@gentoo.org>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or the version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "SettingsNames.h"

namespace Common {

QString SettingsNames::realNameKey = QLatin1String("identity.realName");
QString SettingsNames::addressKey = QLatin1String("identity.address");
QString SettingsNames::msaMethodKey = QLatin1String("msa.method");
QString SettingsNames::methodSMTP = QLatin1String("SMTP");
QString SettingsNames::methodSENDMAIL = QLatin1String("sendmail");
QString SettingsNames::smtpHostKey = QLatin1String("msa.smtp.host");
QString SettingsNames::smtpPortKey = QLatin1String("msa.smtp.port");
QString SettingsNames::smtpAuthKey = QLatin1String("msa.smtp.auth");
QString SettingsNames::smtpUserKey = QLatin1String("msa.smtp.auth.user");
QString SettingsNames::smtpPassKey = QLatin1String("msa.smtp.auth.pass");
QString SettingsNames::sendmailKey = QLatin1String("msa.sendmail");
QString SettingsNames::sendmailDefaultCmd = QLatin1String("sendmail -bm -oi");
QString SettingsNames::imapMethodKey = QLatin1String("imap.method");
QString SettingsNames::methodTCP = QLatin1String("TCP");
QString SettingsNames::methodSSL = QLatin1String("SSL");
QString SettingsNames::methodProcess = QLatin1String("process");
QString SettingsNames::imapHostKey = QLatin1String("imap.host");
QString SettingsNames::imapPortKey = QLatin1String("imap.port");
QString SettingsNames::imapStartTlsKey = QLatin1String("imap.starttls");
QString SettingsNames::imapUserKey = QLatin1String("imap.auth.user");
QString SettingsNames::imapPassKey = QLatin1String("imap.auth.pass");
QString SettingsNames::imapProcessKey = QLatin1String("imap.process");
QString SettingsNames::imapStartOffline = QLatin1String("imap.offline");
QString SettingsNames::cacheMetadataKey = QLatin1String("offline.metadataCache");
QString SettingsNames::cacheMetadataMemory = QLatin1String("memory");
QString SettingsNames::cacheMetadataPersistent = QLatin1String("persistent");
QString SettingsNames::cacheOfflineKey = QLatin1String("offline.sync");
QString SettingsNames::cacheOfflineNone = QLatin1String("none");
QString SettingsNames::cacheOfflineXDays = QLatin1String("xDays");
QString SettingsNames::cacheOfflineXMessages = QLatin1String("xMessages");
QString SettingsNames::cacheOfflineAll = QLatin1String("all");
QString SettingsNames::cacheOfflineNumberDaysKey = QLatin1String("offline.sync.days");
QString SettingsNames::cacheOfflineNumberMessagesKey = QLatin1String("offline.sync.messages");
QString SettingsNames::xtConnectCacheDirectory = QLatin1String("xtconnect.cachedir");
QString SettingsNames::xtSyncMailboxList = QLatin1String("xtconnect.listOfMailboxes");
QString SettingsNames::xtDbHost = QLatin1String("xtconnect.db.hostname");
QString SettingsNames::xtDbPort = QLatin1String("xtconnect.db.port");
QString SettingsNames::xtDbDbName = QLatin1String("xtconnect.db.dbname");
QString SettingsNames::xtDbUser = QLatin1String("xtconnect.db.username");
QString SettingsNames::guiMsgListShowThreading = QLatin1String("gui/msgList.showThreading");
QString SettingsNames::appCheckUpdatesEnabled = QLatin1String("app.updates.checkEnabled");
QString SettingsNames::appCheckUpdatesLastTime = QLatin1String("app.updates.lastTime");
QString SettingsNames::knownEmailsKey = QLatin1String("addressBook/knownEmails");

}
