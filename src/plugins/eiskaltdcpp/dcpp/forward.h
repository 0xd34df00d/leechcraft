/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DCPLUSPLUS_DCPP_FORWARD_H_
#define DCPLUSPLUS_DCPP_FORWARD_H_

/** @file
 * This file contains forward declarations for the various DC++ classes
 */

#include <boost/intrusive_ptr.hpp>

namespace dcpp {

class AdcCommand;

class BufferedSocket;

struct ChatMessage;

class CID;

class Client;

class ClientManager;

class ConnectionQueueItem;

class Download;
typedef Download* DownloadPtr;

class FavoriteHubEntry;
typedef FavoriteHubEntry* FavoriteHubEntryPtr;

class FavoriteUser;

class File;

class FinishedFileItem;
typedef boost::intrusive_ptr<FinishedFileItem> FinishedFileItemPtr;

class FinishedUserItem;
typedef boost::intrusive_ptr<FinishedUserItem> FinishedUserItemPtr;

class FinishedManager;

struct HintedUser;

class HubEntry;

class Identity;

class InputStream;

class LogManager;

class OnlineUser;
typedef OnlineUser* OnlineUserPtr;

class QueueItem;

class SearchResult;
typedef boost::intrusive_ptr<SearchResult> SearchResultPtr;

class Socket;
class SocketException;

class StringSearch;

class Transfer;

class UnZFilter;

class Upload;
typedef Upload* UploadPtr;

class User;
typedef boost::intrusive_ptr<User> UserPtr;

class UserCommand;

class UserConnection;
typedef UserConnection* UserConnectionPtr;

class WindowInfo;
} // namespace dcpp

#endif /*DCPLUSPLUS_DCPP_FORWARD_H_*/
