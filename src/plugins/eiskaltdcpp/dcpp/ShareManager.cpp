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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ShareManager.h"
#include "AdcHub.h"
#include "CryptoManager.h"
#include "ClientManager.h"
#include "LogManager.h"
#include "HashManager.h"
#include "QueueManager.h"
#include "UploadManager.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "File.h"
#include "FilteredFile.h"
#include "BZUtils.h"
#include "Wildcards.h"
#include "Transfer.h"
#include "UserConnection.h"
#include "Download.h"
#include "HashBloom.h"
#include "SearchResult.h"
#include "version.h"
#ifdef WITH_DHT
#include "dht/IndexManager.h"
#endif
#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#endif

#include <limits>

namespace dcpp {
#ifdef WITH_DHT
void ShareManager::publish() {
    Lock l(cs);
    dht::IndexManager::getInstance()->createPublishQueue(tthIndex);
}
#endif
ShareManager::ShareManager() : hits(0), xmlListLen(0), bzXmlListLen(0),
    xmlDirty(true), forceXmlRefresh(false), refreshDirs(false), update(false), initial(true), listN(0), refreshing(false),
    lastXmlUpdate(0), lastFullUpdate(GET_TICK()), bloom(1<<20)
{
    SettingsManager::getInstance()->addListener(this);
    TimerManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);
    HashManager::getInstance()->addListener(this);
}

ShareManager::~ShareManager() {
    SettingsManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
    QueueManager::getInstance()->removeListener(this);
    HashManager::getInstance()->removeListener(this);

    join();

        if(bzXmlRef.get()) {
                bzXmlRef.reset();
                File::deleteFile(getBZXmlFile());
        }
}

ShareManager::Directory::Directory(const string& aName, const ShareManager::Directory::Ptr& aParent) :
    size(0),
    name(aName),
    parent(aParent.get()),
    fileTypes(1 << SearchManager::TYPE_DIRECTORY)
{
}

string ShareManager::Directory::getADCPath() const throw() {
    if(!getParent())
        return '/' + name + '/';
    return getParent()->getADCPath() + name + '/';
}

string ShareManager::Directory::getFullName() const throw() {
    if(!getParent())
        return getName() + '\\';
    return getParent()->getFullName() + getName() + '\\';
}

void ShareManager::Directory::addType(uint32_t type) throw() {
    if(!hasType(type)) {
        fileTypes |= (1 << type);
        if(getParent())
            getParent()->addType(type);
    }
}

string ShareManager::Directory::getRealPath(const std::string& path) const throw(ShareException) {
    if(getParent()) {
        return getParent()->getRealPath(getName() + PATH_SEPARATOR_STR + path);
    } else {
        return ShareManager::getInstance()->findRealRoot(getName(), path);
    }
}

string ShareManager::findRealRoot(const string& virtualRoot, const string& virtualPath) const throw(ShareException) {
    for(StringMap::const_iterator i = shares.begin(); i != shares.end(); ++i) {
        if(Util::stricmp(i->second, virtualRoot) == 0) {
            std::string name = i->first + virtualPath;
            dcdebug("Matching %s\n", name.c_str());
//                      if(FileFindIter(name) != FileFindIter())
                        if (File::getSize(name) != -1)//NOTE: see core 0.750
                return name;
            }
        }

    throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
}

int64_t ShareManager::Directory::getSize() const throw() {
    int64_t tmp = size;
    for(Map::const_iterator i = directories.begin(); i != directories.end(); ++i)
        tmp+=i->second->getSize();
    return tmp;
}

string ShareManager::toVirtual(const TTHValue& tth) const throw(ShareException) {
    if(tth == bzXmlRoot) {
        return Transfer::USER_LIST_NAME_BZ;
    } else if(tth == xmlRoot) {
        return Transfer::USER_LIST_NAME;
    }

    Lock l(cs);
    HashFileMap::const_iterator i = tthIndex.find(tth);
    if(i != tthIndex.end()) {
        return i->second->getADCPath();
    } else {
        throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
    }
}

string ShareManager::toReal(const string& virtualFile) throw(ShareException) {
    Lock l(cs);
    if(virtualFile == "MyList.DcLst") {
        throw ShareException("NMDC-style lists no longer supported, please upgrade your client");
    } else if(virtualFile == Transfer::USER_LIST_NAME_BZ || virtualFile == Transfer::USER_LIST_NAME) {
        generateXmlList();
        return getBZXmlFile();
    }

    return findFile(virtualFile)->getRealPath();
}

StringList ShareManager::getRealPaths(const string& virtualPath) throw(ShareException) {
        if(virtualPath.empty())
                throw ShareException("empty virtual path");

        StringList ret;

        Lock l(cs);

        if(*(virtualPath.end() - 1) == '/') {
                // directory
                Directory::Ptr d = splitVirtual(virtualPath).first;

                // imitate Directory::getRealPath
                if(d->getParent()) {
                        ret.push_back(d->getParent()->getRealPath(d->getName()));
                } else {
                        for(StringMap::const_iterator i = shares.begin(); i != shares.end(); ++i) {
                                if(Util::stricmp(i->second, d->getName()) == 0) {
                                        // remove the trailing path sep
                                        if(FileFindIter(i->first.substr(0, i->first.size() - 1)) != FileFindIter()) {
                                                ret.push_back(i->first);
                                        }
                                }
                        }
                }

        } else {
                // file
                ret.push_back(toReal(virtualPath));
        }

        return ret;
}

TTHValue ShareManager::getTTH(const string& virtualFile) const throw(ShareException) {
    Lock l(cs);
    if(virtualFile == Transfer::USER_LIST_NAME_BZ) {
        return bzXmlRoot;
    } else if(virtualFile == Transfer::USER_LIST_NAME) {
        return xmlRoot;
    }

    return findFile(virtualFile)->getTTH();
}

MemoryInputStream* ShareManager::getTree(const string& virtualFile) const {
    TigerTree tree;
    if(virtualFile.compare(0, 4, "TTH/") == 0) {
        if(!HashManager::getInstance()->getTree(TTHValue(virtualFile.substr(4)), tree))
            return 0;
    } else {
        try {
            TTHValue tth = getTTH(virtualFile);
            HashManager::getInstance()->getTree(tth, tree);
        } catch(const Exception&) {
            return 0;
        }
    }

    ByteVector buf = tree.getLeafData();
    return new MemoryInputStream(&buf[0], buf.size());
}

AdcCommand ShareManager::getFileInfo(const string& aFile) throw(ShareException) {
    if(aFile == Transfer::USER_LIST_NAME) {
        generateXmlList();
        AdcCommand cmd(AdcCommand::CMD_RES);
        cmd.addParam("FN", aFile);
        cmd.addParam("SI", Util::toString(xmlListLen));
        cmd.addParam("TR", xmlRoot.toBase32());
        return cmd;
    } else if(aFile == Transfer::USER_LIST_NAME_BZ) {
        generateXmlList();

        AdcCommand cmd(AdcCommand::CMD_RES);
        cmd.addParam("FN", aFile);
        cmd.addParam("SI", Util::toString(bzXmlListLen));
        cmd.addParam("TR", bzXmlRoot.toBase32());
        return cmd;
    }

    if(aFile.compare(0, 4, "TTH/") != 0)
        throw ShareException(UserConnection::FILE_NOT_AVAILABLE);

    TTHValue val(aFile.substr(4));
    Lock l(cs);
    HashFileIter i = tthIndex.find(val);
    if(i == tthIndex.end()) {
        throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
    }

    const Directory::File& f = *i->second;
    AdcCommand cmd(AdcCommand::CMD_RES);
    cmd.addParam("FN", f.getADCPath());
    cmd.addParam("SI", Util::toString(f.getSize()));
    cmd.addParam("TR", f.getTTH().toBase32());
    return cmd;
}

pair<ShareManager::Directory::Ptr, string> ShareManager::splitVirtual(const string& virtualPath) const throw(ShareException) {
        if(virtualPath.empty() || virtualPath[0] != '/') {
        throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
    }

        string::size_type i = virtualPath.find('/', 1);
    if(i == string::npos || i == 1) {
        throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
    }

        DirList::const_iterator dmi = getByVirtual( virtualPath.substr(1, i - 1));
    if(dmi == directories.end()) {
        throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
    }

    Directory::Ptr d = *dmi;

    string::size_type j = i + 1;
        while((i = virtualPath.find('/', j)) != string::npos) {
                Directory::MapIter mi = d->directories.find(virtualPath.substr(j, i - j));
        j = i + 1;
        if(mi == d->directories.end())
            throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
        d = mi->second;
    }

        return make_pair(d, virtualPath.substr(j));
}

ShareManager::Directory::File::Set::const_iterator ShareManager::findFile(const string& virtualFile) const throw(ShareException) {
        if(virtualFile.compare(0, 4, "TTH/") == 0) {
                HashFileMap::const_iterator i = tthIndex.find(TTHValue(virtualFile.substr(4)));
                if(i == tthIndex.end()) {
                        throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
                }
                return i->second;
        }

        pair<Directory::Ptr, string> v = splitVirtual(virtualFile);
        Directory::File::Set::const_iterator it = find_if(v.first->files.begin(), v.first->files.end(),
                Directory::File::StringComp(v.second));
        if(it == v.first->files.end())
        throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
    return it;
}

string ShareManager::validateVirtual(const string& aVirt) const throw() {
    string tmp = aVirt;
    string::size_type idx = 0;

    while( (idx = tmp.find_first_of("\\/"), idx) != string::npos) {
        tmp[idx] = '_';
    }
    return tmp;
}

bool ShareManager::hasVirtual(const string& virtualName) const throw() {
    Lock l(cs);
    return getByVirtual(virtualName) != directories.end();
}

void ShareManager::load(SimpleXML& aXml) {
    Lock l(cs);

    aXml.resetCurrentChild();
    if(aXml.findChild("Share")) {
        aXml.stepIn();
        while(aXml.findChild("Directory")) {
            string realPath = aXml.getChildData();
            if(realPath.empty()) {
                continue;
            }
            // make sure realPath ends with a PATH_SEPARATOR
            if(realPath[realPath.size() - 1] != PATH_SEPARATOR) {
                realPath += PATH_SEPARATOR;
            }

            const string& virtualName = aXml.getChildAttrib("Virtual");
            string vName = validateVirtual(virtualName.empty() ? Util::getLastDir(realPath) : virtualName);
            shares.insert(std::make_pair(realPath, vName));
            if(getByVirtual(vName) == directories.end()) {
                directories.push_back(Directory::create(vName));
            }
        }
        aXml.stepOut();
    }
}

static const string SDIRECTORY = "Directory";
static const string SFILE = "File";
static const string SNAME = "Name";
static const string SSIZE = "Size";
static const string STTH = "TTH";

struct ShareLoader : public SimpleXMLReader::CallBack {
    ShareLoader(ShareManager::DirList& aDirs) : dirs(aDirs), cur(0), depth(0) { }
    virtual void startTag(const string& name, StringPairList& attribs, bool simple) {
        if(name == SDIRECTORY) {
            const string& name = getAttrib(attribs, SNAME, 0);
            if(!name.empty()) {
                if(depth == 0) {
                    for(ShareManager::DirList::iterator i = dirs.begin(); i != dirs.end(); ++i) {
                        if(Util::stricmp((*i)->getName(), name) == 0) {
                            cur = *i;
                            break;
                        }
                    }
                } else if(cur) {
                    cur = ShareManager::Directory::create(name, cur);
                    cur->getParent()->directories[cur->getName()] = cur;
                }
            }

            if(simple) {
                if(cur) {
                    cur = cur->getParent();
                }
            } else {
                depth++;
            }
        } else if(cur && name == SFILE) {
            const string& fname = getAttrib(attribs, SNAME, 0);
            const string& size = getAttrib(attribs, SSIZE, 1);
            const string& root = getAttrib(attribs, STTH, 2);
            if(fname.empty() || size.empty() || (root.size() != 39)) {
                dcdebug("Invalid file found: %s\n", fname.c_str());
                return;
            }
            cur->files.insert(ShareManager::Directory::File(fname, Util::toInt64(size), cur, TTHValue(root)));
        }
    }
    virtual void endTag(const string& name, const string&) {
        if(name == SDIRECTORY) {
            depth--;
            if(cur) {
                cur = cur->getParent();
            }
        }
    }

private:
    ShareManager::DirList& dirs;

    ShareManager::Directory::Ptr cur;
    size_t depth;
};

bool ShareManager::loadCache() throw() {
    try {
        ShareLoader loader(directories);
                SimpleXMLReader xml(&loader);

        dcpp::File ff(Util::getPath(Util::PATH_USER_CONFIG) + "files.xml.bz2", dcpp::File::READ, dcpp::File::OPEN);
        FilteredInputStream<UnBZFilter, false> f(&ff);

                xml.parse(f);

        for(DirList::const_iterator i = directories.begin(); i != directories.end(); ++i) {
            const Directory::Ptr& d = *i;
            updateIndices(*d);
        }

        return true;
    } catch(const Exception& e) {
        dcdebug("%s\n", e.getError().c_str());
    }
    return false;
}

void ShareManager::save(SimpleXML& aXml) {
    Lock l(cs);

    aXml.addTag("Share");
    aXml.stepIn();
    for(StringMapIter i = shares.begin(); i != shares.end(); ++i) {
        aXml.addTag("Directory", i->first);
        aXml.addChildAttrib("Virtual", i->second);
    }
    aXml.stepOut();
}

void ShareManager::addDirectory(const string& realPath, const string& virtualName) throw(ShareException) {
    if(realPath.empty() || virtualName.empty()) {
        throw ShareException(_("No directory specified"));
    }

    if (!checkHidden(realPath)) {
        throw ShareException(_("Directory is hidden"));
    }

    if(Util::stricmp(SETTING(TEMP_DOWNLOAD_DIRECTORY), realPath) == 0) {
        throw ShareException(_("The temporary download directory cannot be shared"));
    }
    list<string> removeMap;
    {
        Lock l(cs);

        for(StringMapIter i = shares.begin(); i != shares.end(); ++i) {
            if(Util::strnicmp(realPath, i->first, i->first.length()) == 0) {
                // Trying to share an already shared directory
                //throw ShareException(_("Directory already shared"));
                removeMap.push_front(i->first);
            } else if(Util::strnicmp(realPath, i->first, realPath.length()) == 0) {
                // Trying to share a parent directory
                //throw ShareException(_("Remove all subdirectories before adding this one"));
                removeMap.push_front(i->first);
            }
        }
    }
    for(list<string>::const_iterator i = removeMap.begin(); i != removeMap.end(); i++) {
        removeDirectory(*i);
    }


    HashManager::HashPauser pauser;

    Directory::Ptr dp = buildTree(realPath, Directory::Ptr());

    string vName = validateVirtual(virtualName);
    dp->setName(vName);

    {
        Lock l(cs);

        shares.insert(std::make_pair(realPath, vName));
        updateIndices(*merge(dp));

        setDirty();
    }
}

ShareManager::Directory::Ptr ShareManager::merge(const Directory::Ptr& directory) {
    for(DirList::iterator i = directories.begin(); i != directories.end(); ++i) {
        if(Util::stricmp((*i)->getName(), directory->getName()) == 0) {
            dcdebug("Merging directory %s\n", directory->getName().c_str());
            (*i)->merge(directory);
            return *i;
        }
    }

    dcdebug("Adding new directory %s\n", directory->getName().c_str());

    directories.push_back(directory);
    return directory;
}

void ShareManager::Directory::merge(const Directory::Ptr& source) {
    for(MapIter i = source->directories.begin(); i != source->directories.end(); ++i) {
        Directory::Ptr subSource = i->second;

        MapIter ti = directories.find(subSource->getName());
        if(ti == directories.end()) {
            if(findFile(subSource->getName()) != files.end()) {
                dcdebug("File named the same as directory");
            } else {
                directories.insert(std::make_pair(subSource->getName(), subSource));
                                subSource->parent = this;
            }
        } else {
            Directory::Ptr subTarget = ti->second;
            subTarget->merge(subSource);
        }
    }

    // All subdirs either deleted or moved to target...
    source->directories.clear();

    for(File::Set::iterator i = source->files.begin(); i != source->files.end(); ++i) {
        if(findFile(i->getName()) == files.end()) {
            if(directories.find(i->getName()) != directories.end()) {
                dcdebug("Directory named the same as file");
            } else {
                                std::pair<File::Set::iterator, bool> added = files.insert(*i);
                                if(added.second) {
                                        const_cast<File&>(*added.first).setParent(this);
                                }
            }
        }
    }
}

void ShareManager::removeDirectory(const string& realPath) {
    if(realPath.empty())
        return;

    HashManager::getInstance()->stopHashing(realPath);

    Lock l(cs);

    StringMapIter i = shares.find(realPath);
    if(i == shares.end()) {
        return;
    }

    std::string vName = i->second;
    for(DirList::iterator j = directories.begin(); j != directories.end(); ) {
        if(Util::stricmp((*j)->getName(), vName) == 0) {
            directories.erase(j++);
        } else {
            ++j;
        }
    }

    shares.erase(i);

    HashManager::HashPauser pauser;

    // Readd all directories with the same vName
    for(i = shares.begin(); i != shares.end(); ++i) {
        if(Util::stricmp(i->second, vName) == 0 && checkHidden(i->first)) {
            Directory::Ptr dp = buildTree(i->first, 0);
            dp->setName(i->second);
            merge(dp);
        }
    }

    rebuildIndices();
    setDirty();
}

void ShareManager::renameDirectory(const string& realPath, const string& virtualName) throw(ShareException) {
    removeDirectory(realPath);
    addDirectory(realPath, virtualName);
}

ShareManager::DirList::const_iterator ShareManager::getByVirtual(const string& virtualName) const throw() {
    for(DirList::const_iterator i = directories.begin(); i != directories.end(); ++i) {
        if(Util::stricmp((*i)->getName(), virtualName) == 0) {
            return i;
        }
    }
    return directories.end();
}

int64_t ShareManager::getShareSize(const string& realPath) const throw() {
    Lock l(cs);
    dcassert(realPath.size()>0);
    StringMap::const_iterator i = shares.find(realPath);

    if(i != shares.end()) {
        DirList::const_iterator j = getByVirtual(i->second);
        if(j != directories.end()) {
            return (*j)->getSize();
        }
    }
    return -1;
}

int64_t ShareManager::getShareSize() const throw() {
    Lock l(cs);
    int64_t tmp = 0;
    for(HashFileMap::const_iterator i = tthIndex.begin(); i != tthIndex.end(); ++i) {
        tmp += i->second->getSize();
    }
    return tmp;
}

size_t ShareManager::getSharedFiles() const throw() {
    Lock l(cs);
    return tthIndex.size();
}

ShareManager::Directory::Ptr ShareManager::buildTree(const string& aName, const Directory::Ptr& aParent) {
    Directory::Ptr dir = Directory::create(Util::getLastDir(aName), aParent);

    Directory::File::Set::iterator lastFileIter = dir->files.begin();

    FileFindIter end;
    const string l_skip_list = SETTING(SKIPLIST_SHARE);
#ifdef _WIN32
    for(FileFindIter i(aName + "*"); i != end; ++i) {
#else
    //the fileiter just searches directorys for now, not sure if more
    //will be needed later
    //for(FileFindIter i(aName + "*"); i != end; ++i) {
    for(FileFindIter i(aName); i != end; ++i) {
#endif
        string name = i->getFileName();
        if(name.empty()) {
            LogManager::getInstance()->message(str(F_("Invalid file name found while hashing folder %1%") % Util::addBrackets(aName)));
            continue;
        }

        if(name == "." || name == "..")
            continue;
        if(!BOOLSETTING(SHARE_HIDDEN) && i->isHidden())
            continue;
        if(!BOOLSETTING(FOLLOW_LINKS) && i->isLink())
            continue;

        int64_t size = i->getSize();
        string fileName = aName + name;

        if (l_skip_list.size())
        {
            if (Wildcard::patternMatch(fileName , l_skip_list, '|'))
            {
                LogManager::getInstance()->message(str(F_("User has chosen not to share file: %1% (Size: %2%)")
                % Util::addBrackets(fileName) % Util::formatBytes(size)));
                continue;
            }
        }
        if(i->isDirectory()) {
            string newName = aName + name + PATH_SEPARATOR;
            if((Util::stricmp(newName, SETTING(TEMP_DOWNLOAD_DIRECTORY)) != 0)
                    && (Util::stricmp(newName, Util::getPath(Util::PATH_USER_CONFIG)) != 0)
                    && (Util::stricmp(newName, SETTING(LOG_DIRECTORY)) != 0)) {
                dir->directories[name] = buildTree(newName, dir);
            }
        } else {
            // Not a directory, assume it's a file...make sure we're not sharing the settings file...
            const string l_ext = Util::getFileExt(name);
            if ((name != "Thumbs.db") &&
                (name != "desktop.ini") &&
                (name != "folder.htt")
                ) {
                if (!BOOLSETTING(SHARE_TEMP_FILES) &&
                    (Util::stricmp(l_ext.c_str(), ".dctmp") == 0)) {
                    LogManager::getInstance()->message(str(F_("User has chosen not to share temp file: %1% (Size: %2%)")
                    % Util::addBrackets(fileName) % Util::formatBytes(size)));
                    continue;
                }
                if(Util::stricmp(fileName, SETTING(TLS_PRIVATE_KEY_FILE)) == 0) {
                    continue;
                }
                try {
                    if(HashManager::getInstance()->checkTTH(fileName, size, i->getLastWriteTime()))
                        lastFileIter = dir->files.insert(lastFileIter, Directory::File(name, size, dir, HashManager::getInstance()->getTTH(fileName, size)));
                } catch(const HashException&) {
                }
            }
        }
    }

    return dir;
}

//NOTE: freedcpp [+
#ifdef _WIN32
bool ShareManager::checkHidden(const string& aName) const {
        FileFindIter ff = FileFindIter(aName.substr(0, aName.size() - 1));

        if (ff != FileFindIter()) {
                return (BOOLSETTING(SHARE_HIDDEN) || !ff->isHidden());
        }

        return true;
}

#else // !_WIN32

bool ShareManager::checkHidden(const string& aName) const
{
        // check open a directory
        if (!(FileFindIter(aName) != FileFindIter()))
                return true;

        // check hidden directory
        bool hidden = false;
        string path = aName.substr(0, aName.size() - 1);
        string::size_type i = path.rfind(PATH_SEPARATOR);

        if (i != string::npos)
        {
                string dir = path.substr(i + 1);
                if (dir[0] == '.')
                        hidden = true;
        }

        return (BOOLSETTING(SHARE_HIDDEN) || !hidden);
}
#endif // !_WIN32
//NOTE: freedcpp +]

void ShareManager::updateIndices(Directory& dir) {
    bloom.add(Text::toLower(dir.getName()));

    for(Directory::MapIter i = dir.directories.begin(); i != dir.directories.end(); ++i) {
        updateIndices(*i->second);
    }

    dir.size = 0;

    for(Directory::File::Set::iterator i = dir.files.begin(); i != dir.files.end(); ) {
        updateIndices(dir, i++);
    }
}

void ShareManager::rebuildIndices() {
    tthIndex.clear();
    bloom.clear();

    for(DirList::const_iterator i = directories.begin(); i != directories.end(); ++i) {
        updateIndices(**i);
    }
}

void ShareManager::updateIndices(Directory& dir, const Directory::File::Set::iterator& i) {
    const Directory::File& f = *i;

    HashFileIter j = tthIndex.find(f.getTTH());
    if(j == tthIndex.end()) {
        dir.size+=f.getSize();
    } else {
        if(!SETTING(LIST_DUPES)) {
            try {
            LogManager::getInstance()->message(str(F_("Duplicate file will not be shared: %1% (Size: %2% B) Dupe matched against: %3%")
            % Util::addBrackets(dir.getRealPath(f.getName())) % Util::toString(f.getSize()) % Util::addBrackets(j->second->getParent()->getRealPath(j->second->getName()))));
            dir.files.erase(i);
            } catch (const ShareException&) {
            }
            return;
        }
    }

    dir.addType(getType(f.getName()));

    tthIndex.insert(make_pair(f.getTTH(), i));
    bloom.add(Text::toLower(f.getName()));
#ifdef WITH_DHT
    dht::IndexManager* im = dht::IndexManager::getInstance();
    if(im && im->isTimeForPublishing())
        im->publishFile(f.getTTH(), f.getSize());
#endif
}

void ShareManager::refresh(bool dirs /* = false */, bool aUpdate /* = true */, bool block /* = false */) throw() {
    if(refreshing.exchange(true) == true) {
        LogManager::getInstance()->message(_("File list refresh in progress, please wait for it to finish before trying to refresh again"));
        return;
    }
    UploadManager::getInstance()->updateLimits();

    update = aUpdate;
    refreshDirs = dirs;
    join();
    bool cached = false;
    if(initial) {
        cached = loadCache();
        initial = false;
    }
    try {
        start();
        if(block && !cached) {
            join();
        } else {
            setThreadPriority(Thread::LOW);
        }
    } catch(const ThreadException& e) {
        LogManager::getInstance()->message(str(F_("File list refresh failed: %1%") % e.getError()));
    }
}

StringPairList ShareManager::getDirectories() const throw() {
    Lock l(cs);
    StringPairList ret;
    for(StringMap::const_iterator i = shares.begin(); i != shares.end(); ++i) {
        ret.push_back(make_pair(i->second, i->first));
    }
    return ret;
}

int ShareManager::run() {

    StringPairList dirs = getDirectories();
    // Don't need to refresh if no directories are shared
    if(dirs.empty())
        refreshDirs = false;

    if(refreshDirs) {
        HashManager::HashPauser pauser;
        LogManager::getInstance()->message(_("File list refresh initiated"));

        lastFullUpdate = GET_TICK();

        DirList newDirs;
        for(StringPairIter i = dirs.begin(); i != dirs.end(); ++i) {
            if (checkHidden(i->second)) {
                Directory::Ptr dp = buildTree(i->second, Directory::Ptr());
                dp->setName(i->first);
                newDirs.push_back(dp);
            }
        }

        {
            Lock l(cs);
            directories.clear();

            for(DirList::iterator i = newDirs.begin(); i != newDirs.end(); ++i) {
                merge(*i);
            }

            rebuildIndices();
        }
        refreshDirs = false;

        LogManager::getInstance()->message(_("File list refresh finished"));
    }

    if(update) {
        ClientManager::getInstance()->infoUpdated();
    }
    refreshing = false;
#ifdef WITH_DHT
    dht::IndexManager* im = dht::IndexManager::getInstance();
    if(im && im->isTimeForPublishing())
        im->setNextPublishing();
#endif
    return 0;
}

void ShareManager::getBloom(ByteVector& v, size_t k, size_t m, size_t h) const {
    dcdebug("Creating bloom filter, k=%u, m=%u, h=%u\n",
            static_cast<unsigned int>(k), static_cast<unsigned int>(m), static_cast<unsigned int>(h));
    Lock l(cs);

    HashBloom bloom;
    bloom.reset(k, m, h);
    for(HashFileMap::const_iterator i = tthIndex.begin(); i != tthIndex.end(); ++i) {
        bloom.add(i->first);
    }
    bloom.copy_to(v);
}

void ShareManager::generateXmlList() {
    Lock l(cs);
    if(forceXmlRefresh || (xmlDirty && (lastXmlUpdate + 15 * 60 * 1000 < GET_TICK() || lastXmlUpdate < lastFullUpdate))) {
        listN++;

        try {
            string tmp2;
            string indent;

            string newXmlName = Util::getPath(Util::PATH_USER_CONFIG) + "files" + Util::toString(listN) + ".xml.bz2";
            {
                File f(newXmlName, File::WRITE, File::TRUNCATE | File::CREATE);
                // We don't care about the leaves...
                CalcOutputStream<TTFilter<1024*1024*1024>, false> bzTree(&f);
                FilteredOutputStream<BZFilter, false> bzipper(&bzTree);
                CountOutputStream<false> count(&bzipper);
                CalcOutputStream<TTFilter<1024*1024*1024>, false> newXmlFile(&count);

                newXmlFile.write(SimpleXML::utf8Header);
                newXmlFile.write("<FileListing Version=\"1\" CID=\"" + ClientManager::getInstance()->getMe()->getCID().toBase32() + "\" Base=\"/\" Generator=\"" APPNAME " " VERSIONSTRING "\">\r\n");
                for(DirList::const_iterator i = directories.begin(); i != directories.end(); ++i) {
                    (*i)->toXml(newXmlFile, indent, tmp2, true);
                }
                newXmlFile.write("</FileListing>");
                newXmlFile.flush();

                xmlListLen = count.getCount();

                newXmlFile.getFilter().getTree().finalize();
                bzTree.getFilter().getTree().finalize();

                xmlRoot = newXmlFile.getFilter().getTree().getRoot();
                bzXmlRoot = bzTree.getFilter().getTree().getRoot();
            }
            const string XmlListFileName = Util::getPath(Util::PATH_USER_CONFIG) + "files.xml.bz2";
            if(bzXmlRef.get()) {
                bzXmlRef.reset();
                try {
                    File::renameFile(XmlListFileName, XmlListFileName + ".bak");
                } catch(const FileException&) { }
            }

            try {
                File::renameFile(newXmlName, XmlListFileName);
                newXmlName = XmlListFileName;
            } catch(const FileException&) {
                // Ignore, this is for caching only...
            }
            try {
                File::copyFile(XmlListFileName, XmlListFileName + ".bak");
            } catch(const FileException&) { }
            bzXmlRef = auto_ptr<File>(new File(newXmlName, File::READ, File::OPEN));
            setBZXmlFile(newXmlName);
            bzXmlListLen = File::getSize(newXmlName);
            LogManager::getInstance()->message(str(F_("File list %1% generated") % Util::addBrackets(bzXmlFile)));
        } catch(const Exception&) {
            // No new file lists...
        }

        xmlDirty = false;
        forceXmlRefresh = false;
        lastXmlUpdate = GET_TICK();
    }
}

MemoryInputStream* ShareManager::generatePartialList(const string& dir, bool recurse) const {
    if(dir[0] != '/' || dir[dir.size()-1] != '/')
        return 0;

    string xml = SimpleXML::utf8Header;
    string tmp;
    xml += "<FileListing Version=\"1\" CID=\"" + ClientManager::getInstance()->getMe()->getCID().toBase32() + "\" Base=\"" + SimpleXML::escape(dir, tmp, false) + "\" Generator=\"" APPNAME " " VERSIONSTRING "\">\r\n";
    StringOutputStream sos(xml);
    string indent = "\t";

    Lock l(cs);
    if(dir == "/") {
        for(DirList::const_iterator i = directories.begin(); i != directories.end(); ++i) {
            tmp.clear();
            (*i)->toXml(sos, indent, tmp, recurse);
        }
    } else {
        string::size_type i = 1, j = 1;

        Directory::Ptr root;

        bool first = true;
        while( (i = dir.find('/', j)) != string::npos) {
            if(i == j) {
                j++;
                continue;
            }

            if(first) {
                first = false;
                DirList::const_iterator it = getByVirtual(dir.substr(j, i-j));

                if(it == directories.end())
                    return 0;
                root = *it;

            } else {
                Directory::Map::const_iterator it2 = root->directories.find(dir.substr(j, i-j));
                if(it2 == root->directories.end()) {
                    return 0;
                }
                root = it2->second;
            }
            j = i + 1;
        }

        if(!root)
            return 0;

        for(Directory::Map::const_iterator it2 = root->directories.begin(); it2 != root->directories.end(); ++it2) {
            it2->second->toXml(sos, indent, tmp, recurse);
        }
        root->filesToXml(sos, indent, tmp);
    }

    xml += "</FileListing>";
    return new MemoryInputStream(xml);
}

#define LITERAL(n) n, sizeof(n)-1
void ShareManager::Directory::toXml(OutputStream& xmlFile, string& indent, string& tmp2, bool fullList) const {
    xmlFile.write(indent);
    xmlFile.write(LITERAL("<Directory Name=\""));
    xmlFile.write(SimpleXML::escape(name, tmp2, true));

    if(fullList) {
        xmlFile.write(LITERAL("\">\r\n"));

        indent += '\t';
        for(Map::const_iterator i = directories.begin(); i != directories.end(); ++i) {
            i->second->toXml(xmlFile, indent, tmp2, fullList);
        }

        filesToXml(xmlFile, indent, tmp2);

        indent.erase(indent.length()-1);
        xmlFile.write(indent);
        xmlFile.write(LITERAL("</Directory>\r\n"));
    } else {
        if(directories.empty() && files.empty()) {
            xmlFile.write(LITERAL("\" />\r\n"));
        } else {
            xmlFile.write(LITERAL("\" Incomplete=\"1\" />\r\n"));
        }
    }
}

void ShareManager::Directory::filesToXml(OutputStream& xmlFile, string& indent, string& tmp2) const {
    for(Directory::File::Set::const_iterator i = files.begin(); i != files.end(); ++i) {
        const Directory::File& f = *i;

        xmlFile.write(indent);
        xmlFile.write(LITERAL("<File Name=\""));
        xmlFile.write(SimpleXML::escape(f.getName(), tmp2, true));
        xmlFile.write(LITERAL("\" Size=\""));
        xmlFile.write(Util::toString(f.getSize()));
        xmlFile.write(LITERAL("\" TTH=\""));
        tmp2.clear();
        xmlFile.write(f.getTTH().toBase32(tmp2));
        xmlFile.write(LITERAL("\"/>\r\n"));
    }
}

// These ones we can look up as ints (4 bytes...)...

static const char* typeAudio[] = { ".mp3", ".mp2", ".mid", ".wav", ".ogg", ".wma", ".669", ".aac", ".aif", ".amf", ".ams", ".ape", ".dbm", ".dmf", ".dsm", ".far", ".mdl", ".med", ".mod", ".mol", ".mp1", ".mpa", ".mpc", ".mpp", ".mtm", ".nst", ".okt", ".psm", ".ptm", ".rmi", ".s3m", ".stm", ".ult", ".umx", ".wow" };
static const char* typeCompressed[] = { ".rar", ".zip", ".ace", ".arj", ".hqx", ".lha", ".sea", ".tar", ".tgz", ".uc2" };
static const char* typeDocument[] = { ".htm", ".doc", ".txt", ".nfo", ".pdf", ".chm", ".rtf",
                                      ".xls", ".ppt", ".odt", ".ods", ".odf", ".odp" };
static const char* typeExecutable[] = { ".exe", ".com", ".msi" };
static const char* typePicture[] = { ".jpg", ".gif", ".png", ".eps", ".img", ".pct", ".psp", ".pic", ".tif", ".rle", ".bmp", ".pcx", ".jpe", ".dcx", ".emf", ".ico", ".psd", ".tga", ".wmf", ".xif" };
static const char* typeVideo[] = { ".avi", ".mpg", ".mov", ".flv", ".asf",  ".pxp", ".wmv", ".ogm", ".mkv", ".m1v", ".m2v", ".mpe", ".mps", ".mpv", ".ram", ".vob", ".mp4" };
static const char* typeCDImage[] = {".iso", ".mdf", ".mds", ".nrg", ".vcd", ".bwt", ".ccd", ".cdi", ".pdi", ".cue", ".isz", ".img", ".vc4"};

static const string type2Audio[] = { ".au", ".it", ".ra", ".xm", ".aiff", ".flac", ".midi" };
static const string type2Picture[] = { ".ai", ".ps", ".pict", ".jpeg", ".tiff" };
static const string type2Video[] = { ".rm", ".divx", ".mpeg", ".mp1v", ".mp2v", ".mpv1", ".mpv2", ".qt", ".rv", ".vivo", ".ts", ".ps" };

#define IS_TYPE(x) ( type == (*((uint32_t*)x)) )
#define IS_TYPE2(x) (Util::stricmp(aString.c_str() + aString.length() - x.length(), x.c_str()) == 0)

static bool checkType(const string& aString, int aType) {
    if(aType == SearchManager::TYPE_ANY)
        return true;

    if(aString.length() < 5)
        return false;

    const char* c = aString.c_str() + aString.length() - 3;
    if(!Text::isAscii(c))
        return false;

    uint32_t type = '.' | (Text::asciiToLower(c[0]) << 8) | (Text::asciiToLower(c[1]) << 16) | (((uint32_t)Text::asciiToLower(c[2])) << 24);

    switch(aType) {
    case SearchManager::TYPE_AUDIO:
        {
            for(size_t i = 0; i < (sizeof(typeAudio) / sizeof(typeAudio[0])); i++) {
                if(IS_TYPE(typeAudio[i])) {
                    return true;
                }
            }
            if( IS_TYPE2(type2Audio[0]) || IS_TYPE2(type2Audio[1]) || IS_TYPE2(type2Audio[2]) ) {
                return true;
            }
        }
        break;
    case SearchManager::TYPE_CD_IMAGE:
        for(size_t i = 0; i < (sizeof(typeCDImage) / sizeof(typeCDImage[0])); i++) {
            if(IS_TYPE(typeCDImage[i])) {
                return true;
            }
        }

        break;
    case SearchManager::TYPE_COMPRESSED:
        if( IS_TYPE(typeCompressed[0]) || IS_TYPE(typeCompressed[1]) || IS_TYPE(typeCompressed[2]) ) {
            return true;
        }
        break;
    case SearchManager::TYPE_DOCUMENT:
        if( IS_TYPE(typeDocument[0]) || IS_TYPE(typeDocument[1]) ||
            IS_TYPE(typeDocument[2]) || IS_TYPE(typeDocument[3]) ) {
            return true;
        }
        break;
    case SearchManager::TYPE_EXECUTABLE:
        if(IS_TYPE(typeExecutable[0]) ) {
            return true;
        }
        break;
    case SearchManager::TYPE_PICTURE:
        {
            for(size_t i = 0; i < (sizeof(typePicture) / sizeof(typePicture[0])); i++) {
                if(IS_TYPE(typePicture[i])) {
                    return true;
                }
            }
            if( IS_TYPE2(type2Picture[0]) || IS_TYPE2(type2Picture[1]) || IS_TYPE2(type2Picture[2]) ) {
                return true;
            }
        }
        break;
    case SearchManager::TYPE_VIDEO:
        {
            for(size_t i = 0; i < (sizeof(typeVideo) / sizeof(typeVideo[0])); i++) {
                if(IS_TYPE(typeVideo[i])) {
                    return true;
                }
            }
            if( IS_TYPE2(type2Video[0]) || IS_TYPE2(type2Video[1]) || IS_TYPE2(type2Video[2]) ) {
                return true;
            }
        }
        break;
    default:
        dcassert(0);
        break;
    }
    return false;
}

SearchManager::TypeModes ShareManager::getType(const string& aFileName) const throw() {
    if(aFileName[aFileName.length() - 1] == PATH_SEPARATOR) {
        return SearchManager::TYPE_DIRECTORY;
    }

    if(checkType(aFileName, SearchManager::TYPE_VIDEO))
        return SearchManager::TYPE_VIDEO;
    else if(checkType(aFileName, SearchManager::TYPE_AUDIO))
        return SearchManager::TYPE_AUDIO;
    else if(checkType(aFileName, SearchManager::TYPE_COMPRESSED))
        return SearchManager::TYPE_COMPRESSED;
    else if(checkType(aFileName, SearchManager::TYPE_DOCUMENT))
        return SearchManager::TYPE_DOCUMENT;
    else if(checkType(aFileName, SearchManager::TYPE_EXECUTABLE))
        return SearchManager::TYPE_EXECUTABLE;
    else if(checkType(aFileName, SearchManager::TYPE_PICTURE))
        return SearchManager::TYPE_PICTURE;
    else if(checkType(aFileName, SearchManager::TYPE_CD_IMAGE))
        return SearchManager::TYPE_CD_IMAGE;

    return SearchManager::TYPE_ANY;
}

/**
 * Alright, the main point here is that when searching, a search string is most often found in
 * the filename, not directory name, so we want to make that case faster. Also, we want to
 * avoid changing StringLists unless we absolutely have to --> this should only be done if a string
 * has been matched in the directory name. This new stringlist should also be used in all descendants,
 * but not the parents...
 */
void ShareManager::Directory::search(SearchResultList& aResults, StringSearch::List& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) const throw() {
    // Skip everything if there's nothing to find here (doh! =)
    if(!hasType(aFileType))
        return;

    StringSearch::List* cur = &aStrings;
    auto_ptr<StringSearch::List> newStr;

    // Find any matches in the directory name
    for(StringSearch::List::const_iterator k = aStrings.begin(); k != aStrings.end(); ++k) {
        if(k->match(name)) {
            if(!newStr.get()) {
                newStr = auto_ptr<StringSearch::List>(new StringSearch::List(aStrings));
            }
            newStr->erase(remove(newStr->begin(), newStr->end(), *k), newStr->end());
        }
    }

    if(newStr.get() != 0) {
        cur = newStr.get();
    }

    bool sizeOk = (aSearchType != SearchManager::SIZE_ATLEAST) || (aSize == 0);
    if( (cur->empty()) &&
        (((aFileType == SearchManager::TYPE_ANY) && sizeOk) || (aFileType == SearchManager::TYPE_DIRECTORY)) ) {
        // We satisfied all the search words! Add the directory...(NMDC searches don't support directory size)
        SearchResultPtr sr(new SearchResult(SearchResult::TYPE_DIRECTORY, 0, getFullName(), TTHValue()));
        aResults.push_back(sr);
        ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
    }

    if(aFileType != SearchManager::TYPE_DIRECTORY) {
        for(File::Set::const_iterator i = files.begin(); i != files.end(); ++i) {

            if(aSearchType == SearchManager::SIZE_ATLEAST && aSize > i->getSize()) {
                continue;
            } else if(aSearchType == SearchManager::SIZE_ATMOST && aSize < i->getSize()) {
                continue;
            }
            StringSearch::List::iterator j = cur->begin();
            for(; j != cur->end() && j->match(i->getName()); ++j)
                ;   // Empty

            if(j != cur->end())
                continue;

            // Check file type...
            if(checkType(i->getName(), aFileType)) {
                SearchResultPtr sr(new SearchResult(SearchResult::TYPE_FILE, i->getSize(), getFullName() + i->getName(), i->getTTH()));
                aResults.push_back(sr);
                ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
                if(aResults.size() >= maxResults) {
                    break;
                }
            }
        }
    }

    for(Directory::Map::const_iterator l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
        l->second->search(aResults, *cur, aSearchType, aSize, aFileType, aClient, maxResults);
    }
}

void ShareManager::search(SearchResultList& results, const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) throw() {
    Lock l(cs);
    if(aFileType == SearchManager::TYPE_TTH) {
        if(aString.compare(0, 4, "TTH:") == 0) {
            TTHValue tth(aString.substr(4));
            HashFileMap::const_iterator i = tthIndex.find(tth);
            if(i != tthIndex.end()) {
                SearchResultPtr sr(new SearchResult(SearchResult::TYPE_FILE, i->second->getSize(),
                    i->second->getParent()->getFullName() + i->second->getName(), i->second->getTTH()));

                results.push_back(sr);
                ShareManager::getInstance()->addHits(1);
            }
        }
        return;
    }
    StringTokenizer<string> t(Text::toLower(aString), '$');
    StringList& sl = t.getTokens();
    if(!bloom.match(sl))
        return;

    StringSearch::List ssl;
    for(StringList::iterator i = sl.begin(); i != sl.end(); ++i) {
        if(!i->empty()) {
            ssl.push_back(StringSearch(*i));
        }
    }
    if(ssl.empty())
        return;

    for(DirList::const_iterator j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
        (*j)->search(results, ssl, aSearchType, aSize, aFileType, aClient, maxResults);
    }
}

namespace {
    inline uint16_t toCode(char a, char b) { return (uint16_t)a | ((uint16_t)b)<<8; }
}

ShareManager::AdcSearch::AdcSearch(const StringList& params) : include(&includeX), gt(0),
    lt(numeric_limits<int64_t>::max()), hasRoot(false), isDirectory(false)
{
    for(StringIterC i = params.begin(); i != params.end(); ++i) {
        const string& p = *i;
        if(p.length() <= 2)
            continue;

        uint16_t cmd = toCode(p[0], p[1]);
        if(toCode('T', 'R') == cmd) {
            hasRoot = true;
            root = TTHValue(p.substr(2));
            return;
        } else if(toCode('A', 'N') == cmd) {
            includeX.push_back(StringSearch(p.substr(2)));
        } else if(toCode('N', 'O') == cmd) {
            exclude.push_back(StringSearch(p.substr(2)));
        } else if(toCode('E', 'X') == cmd) {
            ext.push_back(p.substr(2));
        } else if(toCode('G', 'R') == cmd) {
           StringList exts = AdcHub::parseSearchExts(Util::toInt(p.substr(2)));
            ext.insert(ext.begin(), exts.begin(), exts.end());
        } else if(toCode('R', 'X') == cmd) {
            noExt.push_back(p.substr(2));
        } else if(toCode('G', 'E') == cmd) {
            gt = Util::toInt64(p.substr(2));
        } else if(toCode('L', 'E') == cmd) {
            lt = Util::toInt64(p.substr(2));
        } else if(toCode('E', 'Q') == cmd) {
            lt = gt = Util::toInt64(p.substr(2));
        } else if(toCode('T', 'Y') == cmd) {
            isDirectory = (p[2] == '2');
        }
    }
}

bool ShareManager::AdcSearch::isExcluded(const string& str) {
    for(StringSearch::List::iterator i = exclude.begin(); i != exclude.end(); ++i) {
        if(i->match(str))
            return true;
    }
    return false;
}

bool ShareManager::AdcSearch::hasExt(const string& name) {
    if(ext.empty())
        return true;
    if(!noExt.empty()) {
        ext = StringList(ext.begin(), set_difference(ext.begin(), ext.end(), noExt.begin(), noExt.end(), ext.begin()));
        noExt.clear();
    }
    for(StringIter i = ext.begin(), iend = ext.end(); i != iend; ++i) {
        if(name.length() >= i->length() && Util::stricmp(name.c_str() + name.length() - i->length(), i->c_str()) == 0)
            return true;
    }
    return false;
}

void ShareManager::Directory::search(SearchResultList& aResults, AdcSearch& aStrings, StringList::size_type maxResults) const throw() {
    StringSearch::List* cur = aStrings.include;
    StringSearch::List* old = aStrings.include;

    auto_ptr<StringSearch::List> newStr;

    // Find any matches in the directory name
    for(StringSearch::List::iterator k = cur->begin(); k != cur->end(); ++k) {
        if(k->match(name) && !aStrings.isExcluded(name)) {
            if(!newStr.get()) {
                newStr = auto_ptr<StringSearch::List>(new StringSearch::List(*cur));
            }
            newStr->erase(remove(newStr->begin(), newStr->end(), *k), newStr->end());
        }
    }

    if(newStr.get() != 0) {
        cur = newStr.get();
    }

    bool sizeOk = (aStrings.gt == 0);
    if( cur->empty() && aStrings.ext.empty() && sizeOk ) {
        // We satisfied all the search words! Add the directory...
        SearchResultPtr sr(new SearchResult(SearchResult::TYPE_DIRECTORY, getSize(), getFullName(), TTHValue()));
        aResults.push_back(sr);
        ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
    }

    if(!aStrings.isDirectory) {
        for(File::Set::const_iterator i = files.begin(); i != files.end(); ++i) {

            if(!(i->getSize() >= aStrings.gt)) {
                continue;
            } else if(!(i->getSize() <= aStrings.lt)) {
                continue;
            }

            if(aStrings.isExcluded(i->getName()))
                continue;

            StringSearch::List::iterator j = cur->begin();
            for(; j != cur->end() && j->match(i->getName()); ++j)
                ;   // Empty

            if(j != cur->end())
                continue;

            // Check file type...
            if(aStrings.hasExt(i->getName())) {

                SearchResultPtr sr(new SearchResult(SearchResult::TYPE_FILE,
                    i->getSize(), getFullName() + i->getName(), i->getTTH()));
                aResults.push_back(sr);
                ShareManager::getInstance()->addHits(1);
                if(aResults.size() >= maxResults) {
                    return;
                }
            }
        }
    }

    for(Directory::Map::const_iterator l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
        l->second->search(aResults, aStrings, maxResults);
    }
    aStrings.include = old;
}

void ShareManager::search(SearchResultList& results, const StringList& params, StringList::size_type maxResults) throw() {
    AdcSearch srch(params);

    Lock l(cs);

    if(srch.hasRoot) {
        HashFileMap::const_iterator i = tthIndex.find(srch.root);
        if(i != tthIndex.end()) {
            SearchResultPtr sr(new SearchResult(SearchResult::TYPE_FILE,
                i->second->getSize(), i->second->getParent()->getFullName() + i->second->getName(),
                i->second->getTTH()));
            results.push_back(sr);
            addHits(1);
        }
        return;
    }

    for(StringSearch::List::const_iterator i = srch.includeX.begin(); i != srch.includeX.end(); ++i) {
        if(!bloom.match(i->getPattern()))
            return;
    }

    for(DirList::const_iterator j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
        (*j)->search(results, srch, maxResults);
    }
}

ShareManager::Directory::Ptr ShareManager::getDirectory(const string& fname) {
    for(StringMapIter mi = shares.begin(); mi != shares.end(); ++mi) {
        if(Util::strnicmp(fname, mi->first, mi->first.length()) == 0) {
            Directory::Ptr d;
            for(DirList::iterator i = directories.begin(); i != directories.end(); ++i) {
                if(Util::stricmp((*i)->getName(), mi->second) == 0) {
                    d = *i;
                }
            }

            if(!d) {
                return Directory::Ptr();
            }

            string::size_type i;
            string::size_type j = mi->first.length();
            while( (i = fname.find(PATH_SEPARATOR, j)) != string::npos) {
                Directory::MapIter dmi = d->directories.find(fname.substr(j, i-j));
                j = i + 1;
                if(dmi == d->directories.end())
                    return Directory::Ptr();
                d = dmi->second;
            }
            return d;
        }
    }
    return Directory::Ptr();
}

void ShareManager::on(QueueManagerListener::FileMoved, const string& n) throw() {
    if(BOOLSETTING(ADD_FINISHED_INSTANTLY)) {
        // Check if finished download is supposed to be shared
        Lock l(cs);
        for(StringMapIter i = shares.begin(); i != shares.end(); i++) {
            if(Util::strnicmp(i->first, n, i->first.size()) == 0 && n[i->first.size() - 1] == PATH_SEPARATOR) {
                try {
                    // Schedule for hashing, it'll be added automatically later on...
                    HashManager::getInstance()->checkTTH(n, File::getSize(n), 0);
                } catch(const Exception&) {
                    // Not a vital feature...
                }
                break;
            }
        }
    }
}

void ShareManager::on(HashManagerListener::TTHDone, const string& fname, const TTHValue& root) throw() {
    Lock l(cs);
    Directory::Ptr d = getDirectory(fname);
    if(d) {
        Directory::File::Set::const_iterator i = d->findFile(Util::getFileName(fname));
        if(i != d->files.end()) {
            if(root != i->getTTH())
                tthIndex.erase(i->getTTH());
            // Get rid of false constness...
            Directory::File* f = const_cast<Directory::File*>(&(*i));
            f->setTTH(root);
            tthIndex.insert(make_pair(f->getTTH(), i));
        } else {
            string name = Util::getFileName(fname);
            int64_t size = File::getSize(fname);
            Directory::File::Set::iterator it = d->files.insert(Directory::File(name, size, d, root)).first;
            updateIndices(*d, it);
        }
        setDirty();
        forceXmlRefresh = true;
    }
}

void ShareManager::on(TimerManagerListener::Minute, uint64_t tick) throw() {
    if (SETTING(AUTO_REFRESH_TIME) > 0) {
        //if (Util::getUpTime() > 5 * 60) { // [+] IRainman: disabling update file list immediately after startup
            if (lastFullUpdate + SETTING(AUTO_REFRESH_TIME) * 60 * 1000 < tick) {
                refresh(true, true);
            }
        //}
    }
}

} // namespace dcpp
