#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <settingsdialog/settingsiteminfo.h>
#include <interfaces/interfaces.h>
#include <plugininterface/guarded.h>
#include <QPair>
#include <QMap>
#include "globals.h"

class QReadWriteLock;

class SettingsManager : public QObject
					  , public ISettings
{
	Q_OBJECT
	Q_INTERFACES (ISettings);

	SettingsManager ();
	~SettingsManager ();

	static SettingsManager *Instance_;
	QMap<QString, SettingsItemInfo> PropertyInfo_;
	static QReadWriteLock *InstanceMutex_;
protected:
	Guarded<int> PendingRequestTimeout_;
	Guarded<int> ClientTimeout_;
	Guarded<int> ConnectTimeout_;
	Guarded<int> KAInterval_;
	Guarded<int> RateControlTimerDelay_;
	Guarded<int> ServerMinPort_;
	Guarded<int> ServerMaxPort_;
	Guarded<int> BlockSize_;
	Guarded<int> MaxBlocksInProgress_;
	Guarded<int> MaxTotalConnections_;
	Guarded<int> RateControlWindowLength_;
	Guarded<int> UploadScheduleInterval_;
	Guarded<int> EndGamePieces_;
	Guarded<int> MaxConnectionsPerPeer_;
	Guarded<int> MinimumRevisitTime_;
	Guarded<int> MaxUploads_;
	Guarded<int> MaxBlocksInMultiMode_;
	Guarded<int> ConsiderableUploadSpeed_;
private:
	void ReadSettings ();
	void WriteSettings ();
	void InitializeMap ();
public:
	static SettingsManager* Instance ();
	void Release ();
	void Flush ();

	int GetPendingRequestTimeout () const;
	void SetPendingRequestTimeout (int);
	int GetClientTimeout () const;
	void SetClientTimeout (int);
	int GetConnectTimeout () const;
	void SetConnectTimeout (int);
	int GetKAInterval () const;
	void SetKAInterval (int);
	int GetRateControlTimerDelay () const;
	void SetRateControlTimerDelay (int);
	int GetServerMinPort () const;
	void SetServerMinPort (int);
	int GetServerMaxPort () const;
	void SetServerMaxPort (int);
	int GetBlockSize () const;
	void SetBlockSize (int);
	int GetMaxBlocksInProgress () const;
	void SetMaxBlocksInProgress (int);
	int GetMaxTotalConnections () const;
	void SetMaxTotalConnections (int);
	int GetRateControlWindowLength () const;
	void SetRateControlWindowLength (int);
	int GetUploadScheduleInterval () const;
	void SetUploadScheduleInterval (int);
	int GetEndGamePieces () const;
	void SetEndGamePieces (int);
	int GetMaxConnectionsPerPeer () const;
	void SetMaxConnectionsPerPeer (int);
	int GetMinimumRevisitTime () const;
	void SetMinimumRevisitTime (int);
	int GetMaxUploads () const;
	void SetMaxUploads (int);
	int GetMaxBlocksInMultiMode () const;
	void SetMaxBlocksInMultiMode (int);
	int GetConsiderableUploadSpeed () const;
	void SetConsiderableUploadSpeed (int);

	SettingsItemInfo GetInfoFor (const QString&) const;

	Q_PROPERTY (int ServerMinPort READ GetServerMinPort WRITE SetServerMinPort);
	Q_PROPERTY (int ServerMaxPort READ GetServerMaxPort WRITE SetServerMaxPort);
	Q_PROPERTY (int PendingRequestTimeout READ GetPendingRequestTimeout WRITE SetPendingRequestTimeout);
	Q_PROPERTY (int ClientTimeout READ GetClientTimeout WRITE SetClientTimeout);
	Q_PROPERTY (int ConnectTimeout READ GetConnectTimeout WRITE SetConnectTimeout);
	Q_PROPERTY (int KAInterval READ GetKAInterval WRITE SetKAInterval);
	Q_PROPERTY (int UploadScheduleInterval READ GetUploadScheduleInterval WRITE SetUploadScheduleInterval);
	Q_PROPERTY (int MinimumRevisitTime READ GetMinimumRevisitTime WRITE SetMinimumRevisitTime);
	Q_PROPERTY (int MaxConnectionsPerPeer READ GetMaxConnectionsPerPeer WRITE SetMaxConnectionsPerPeer);
	Q_PROPERTY (int MaxUploads READ GetMaxUploads WRITE SetMaxUploads);
	Q_PROPERTY (int MaxTotalConnections READ GetMaxTotalConnections WRITE SetMaxTotalConnections);
	Q_PROPERTY (int ConsiderableUploadSpeed READ GetConsiderableUploadSpeed WRITE SetConsiderableUploadSpeed);
	Q_PROPERTY (int BlockSize READ GetBlockSize WRITE SetBlockSize);
	Q_PROPERTY (int MaxBlocksInProgress READ GetMaxBlocksInProgress WRITE SetMaxBlocksInProgress);
	Q_PROPERTY (int MaxBlocksInMultiMode READ GetMaxBlocksInMultiMode WRITE SetMaxBlocksInMultiMode);
	Q_PROPERTY (int EndGamePieces READ GetEndGamePieces WRITE SetEndGamePieces);

	Q_PROPERTY (int RateControlTimerDelay READ GetRateControlTimerDelay WRITE SetRateControlTimerDelay);
	Q_PROPERTY (int RateControlWindowLength READ GetRateControlWindowLength WRITE SetRateControlWindowLength);
};

#endif

