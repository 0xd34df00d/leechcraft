SET( CLIENT_SRC
ADLSearch.cpp
AdcCommand.cpp
AdcHub.cpp
BZUtils.cpp
BufferedSocket.cpp
Client.cpp
ClientManager.cpp
ConnectionManager.cpp
CryptoManager.cpp
DCPlusPlus.cpp
DirectoryListing.cpp
Download.cpp
DownloadManager.cpp
Encoder.cpp
Exception.cpp
FavoriteManager.cpp
File.cpp
FinishedItem.cpp
FinishedManager.cpp
HashBloom.cpp
HashManager.cpp
HttpConnection.cpp
LogManager.cpp
NmdcHub.cpp
QueueItem.cpp
QueueManager.cpp
ResourceManager.cpp
SFVReader.cpp
SSL.cpp
SSLSocket.cpp
SearchManager.cpp
SearchResult.cpp
ServerSocket.cpp
SettingsManager.cpp
ShareManager.cpp
SimpleXML.cpp
Socket.cpp
StringTokenizer.cpp
Text.cpp
Thread.cpp
TigerHash.cpp
TimerManager.cpp
Transfer.cpp
Upload.cpp
UploadManager.cpp
User.cpp
UserConnection.cpp
Util.cpp
ZUtils.cpp
stdinc.cpp
)

SET(LIB
bz2
z
ssl
pthread
m
)
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES (${OPENSSL_INCLUDE_DIR})
add_definitions (-D_REENTRANT -fPIC)
IF (APPLE)
    SET ( LIB ${LIB} crypto iconv)
ENDIF(APPLE)

GET_NATIVE_PRECOMPILED_HEADER(dcpp ./stdinc.h)

ADD_LIBRARY(dcpp ${CLIENT_SRC} ${dcpp_pch})

TARGET_LINK_LIBRARIES(dcpp ${LIB} ${OPENSSL_LIBRARIES} )
ADD_NATIVE_PRECOMPILED_HEADER(dcpp ./stdinc.h)
