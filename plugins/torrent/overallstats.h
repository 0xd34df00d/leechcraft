#ifndef OVERALLSTATS_H
#define OVERALLSTATS_H

struct OverallStats
{
	int NumUploads_,
		NumConnections_,
		SessionDownload_,
		SessionUpload_,
		NumPeers_,
		NumDHTNodes_,
		NumDHTTorrents_;
	float	DownloadRate_,
			UploadRate_;
	unsigned short ListenPort_;
};

#endif

