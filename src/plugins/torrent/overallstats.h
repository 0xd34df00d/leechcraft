#ifndef OVERALLSTATS_H
#define OVERALLSTATS_H
#include <QtGlobal>

struct OverallStats
{
 int NumUploads_,
  NumConnections_,
  NumPeers_,
  NumDHTNodes_,
  NumDHTTorrents_;
 quint64 SessionDownload_,
   SessionUpload_;
 float DownloadRate_,
   UploadRate_;
 unsigned short ListenPort_;
};

#endif

