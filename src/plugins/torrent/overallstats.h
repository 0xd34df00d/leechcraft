#ifndef OVERALLSTATS_H
#define OVERALLSTATS_H
#include <QtGlobal>

struct OverallStats
{
    int NumUploads_,
        NumConnections_,
        NumPeers_,
        NumDHTNodes_,
        NumGlobalDHTNodes_,
        NumDHTTorrents_;
    quint64    SessionDownload_,
            SessionUpload_,
			TotalFailedData_,
			TotalRedundantData_;
    float    DownloadRate_,
            UploadRate_;
    unsigned short ListenPort_;
};

#endif

