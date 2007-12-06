#include <QSettings>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "settingsmanager.h"

SettingsManager *SettingsManager::Instance_ = 0;
QReadWriteLock *SettingsManager::InstanceMutex_ = new QReadWriteLock;

SettingsManager::SettingsManager ()
{
	ReadSettings ();
	InitializeMap ();
}

SettingsManager::~SettingsManager ()
{
}

void SettingsManager::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (Globals::Name);
	settings.beginGroup ("mainsettings");
	PendingRequestTimeout_ = settings.value ("PendingRequestTimeout", 60 * 1000).toInt ();
	ClientTimeout_ = settings.value ("ClientTimeout", 120 * 1000).toInt ();
	ConnectTimeout_ = settings.value ("ConnectTimeout", 60 * 1000).toInt ();
	KAInterval_ = settings.value ("KAInterval", 30 * 1000).toInt ();
	RateControlTimerDelay_ = settings.value ("RateControlTimerDelay", 2000).toInt ();
	ServerMinPort_ = settings.value ("ServerMinPort", 6881).toInt ();
	ServerMaxPort_ = settings.value ("ServerMaxPort", 6889).toInt ();
	BlockSize_ = settings.value ("BlockSize", 16384).toInt ();
	MaxBlocksInProgress_ = settings.value ("MaxBlocksInProgress", 5).toInt ();
	MaxTotalConnections_ = settings.value ("MaxTotalConnections", 250).toInt ();
	RateControlWindowLength_ = settings.value ("RateControlWindowLength", 10).toInt ();
	UploadScheduleInterval_ = settings.value ("UploadScheduleInterval", 10000).toInt ();
	EndGamePieces_ = settings.value ("EndGamePieces", 5).toInt ();
	MaxConnectionsPerPeer_ = settings.value ("MaxConnectionsPerPeer", 1).toInt ();
	MinimumRevisitTime_ = settings.value ("MinimumRevisitTime", 30).toInt ();
	MaxUploads_ = settings.value ("MaxUploads", 20).toInt ();
	MaxBlocksInMultiMode_ = settings.value ("MaxBlocksInMultiMode", 10).toInt ();
	ConsiderableUploadSpeed_ = settings.value ("ConsiderableUploadSpeed", 1024).toInt ();
	settings.endGroup ();
	settings.endGroup ();
}

void SettingsManager::WriteSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (Globals::Name);
	settings.beginGroup ("mainsettings");
	settings.setValue ("PendingRequestTimeout", PendingRequestTimeout_);
	settings.setValue ("ClientTimeout", ClientTimeout_);
	settings.setValue ("ConnectTimeout", ConnectTimeout_);
	settings.setValue ("KAInterval", KAInterval_);
	settings.setValue ("RateControlTimerDelay", RateControlTimerDelay_);
	settings.setValue ("ServerMinPort", ServerMinPort_);
	settings.setValue ("ServerMaxPort", ServerMaxPort_);
	settings.setValue ("BlockSize", BlockSize_);
	settings.setValue ("MaxBlocksInProgress", MaxBlocksInProgress_);
	settings.setValue ("MaxTotalConnections", MaxTotalConnections_);
	settings.setValue ("RateControlWindowLength", RateControlWindowLength_);
	settings.setValue ("UploadScheduleInterval", UploadScheduleInterval_);
	settings.setValue ("EndGamePieces", EndGamePieces_);
	settings.setValue ("MaxConnectionsPerPeer", MaxConnectionsPerPeer_);
	settings.setValue ("MinimumRevisitTime", MinimumRevisitTime_);
	settings.setValue ("MaxUploads", MaxUploads_);
	settings.setValue ("MaxBlocksInMultiMode", MaxBlocksInMultiMode_);
	settings.setValue ("ConsiderableUploadSpeed", ConsiderableUploadSpeed_);
	settings.endGroup ();
	settings.endGroup ();
}

void SettingsManager::InitializeMap ()
{
	SettingsItemInfo serverMinPort = SettingsItemInfo (tr ("Minimal server port"), tr ("Network"));
	serverMinPort.IntRange_ = qMakePair (1025, 65535);
	PropertyInfo_ ["ServerMinPort"] = serverMinPort;

	SettingsItemInfo serverMaxPort = SettingsItemInfo (tr ("Maximal server port"), tr ("Network"));
	serverMaxPort.IntRange_ = qMakePair (1025, 65535);
	PropertyInfo_ ["ServerMaxPort"] = serverMaxPort;

	SettingsItemInfo pendingRequestTimeout (tr ("Pending request timeout"), tr ("Network"), tr ("Timeouts"));
	pendingRequestTimeout.SpinboxSuffix_ = tr (" ms");
	pendingRequestTimeout.SpinboxStep_ = 1000;
	pendingRequestTimeout.IntRange_ = qMakePair (0, 1200000);
	PropertyInfo_ ["PendingRequestTimeout"] = pendingRequestTimeout;

	SettingsItemInfo clientTimeout = SettingsItemInfo (tr ("Client timeout"), tr ("Network"), tr ("Timeouts"));
	clientTimeout.SpinboxSuffix_ = tr (" ms");
	clientTimeout.SpinboxStep_ = 1000;
	clientTimeout.IntRange_ = qMakePair (0, 1200000);
	PropertyInfo_ ["ClientTimeout"] = clientTimeout;

	SettingsItemInfo connectTimeout = SettingsItemInfo (tr ("Connect timeout"), tr ("Network"), tr ("Timeouts"));
	connectTimeout.SpinboxSuffix_ = tr (" ms");
	connectTimeout.SpinboxStep_ = 1000;
	connectTimeout.IntRange_ = qMakePair (0, 1200000);
	PropertyInfo_ ["ConnectTimeout"] = connectTimeout;

	SettingsItemInfo kaInterval = SettingsItemInfo (tr ("Keep alive interval"), tr ("Network"), tr ("Timeouts"));
	kaInterval.SpinboxSuffix_ = tr (" ms");
	kaInterval.SpinboxStep_ = 1000;
	kaInterval.IntRange_ = qMakePair (0, 1200000);
	PropertyInfo_ ["KAInterval"] = kaInterval;

	SettingsItemInfo blockSize = SettingsItemInfo (tr ("Block size"), tr ("Network"), tr ("Performance"));
	blockSize.IntRange_ = qMakePair (4096, 16384);
	blockSize.SpinboxSuffix_ = tr (" bytes");
	blockSize.SpinboxStep_ = 16;
	PropertyInfo_ ["BlockSize"] = blockSize;

	SettingsItemInfo maxBlocksInProgress = SettingsItemInfo (tr ("Max blocks in progress"), tr ("Network"), tr ("Performance"));
	maxBlocksInProgress.IntRange_ = qMakePair (1, 99);
	PropertyInfo_ ["MaxBlocksInProgress"] = maxBlocksInProgress;

	SettingsItemInfo maxTotalConnections = SettingsItemInfo (tr ("Total max connections"), tr ("Network"), tr ("Performance"));
	maxTotalConnections.IntRange_ = qMakePair (5, 1000);
	PropertyInfo_ ["MaxTotalConnections"] = maxTotalConnections;

	SettingsItemInfo uploadScheduleInterval = SettingsItemInfo (tr ("Upload reschedule interval"), tr ("Network"), tr ("Timeouts"));
	uploadScheduleInterval.IntRange_ = qMakePair (100, 600000);
	uploadScheduleInterval.SpinboxSuffix_ = tr (" ms");
	PropertyInfo_ ["UploadScheduleInterval"] = uploadScheduleInterval;

	SettingsItemInfo endGamePieces = SettingsItemInfo (tr ("Pieces required to enter End Game"), tr ("Network"), tr ("Performance"));
	endGamePieces.IntRange_ = qMakePair (3, 20);
	PropertyInfo_ ["EndGamePieces"] = endGamePieces;

	SettingsItemInfo maxConnectionsPerPeer = SettingsItemInfo (tr ("Maximum connections with single peer"), tr ("Network"), tr ("Performance"));
	maxConnectionsPerPeer.IntRange_ = qMakePair (1, 10);
	PropertyInfo_ ["MaxConnectionsPerPeer"] = maxConnectionsPerPeer;

	SettingsItemInfo minimumRevisitTime = SettingsItemInfo (tr ("Minimum time between peer revisits"), tr ("Network"), tr ("Timeouts"));
	minimumRevisitTime.IntRange_ = qMakePair (10, 300);
	minimumRevisitTime.SpinboxSuffix_ = tr (" s");
	PropertyInfo_ ["MinimumRevisitTime"] = minimumRevisitTime;

	SettingsItemInfo maxUploads = SettingsItemInfo (tr ("Maximum simultaneous uploads"), tr ("Network"), tr ("Performance"));
	maxUploads.IntRange_ = qMakePair (1, 1000);
	PropertyInfo_ ["MaxUploads"] = maxUploads;

	SettingsItemInfo maxBlocksInMultiMode = SettingsItemInfo (tr ("Maximum blocks in End Game/Warming Up mode"), tr ("Network"), tr ("Performance"));
	maxBlocksInMultiMode.IntRange_ = qMakePair (1, 200);
	PropertyInfo_ ["MaxBlocksInMultiMode"] = maxBlocksInMultiMode;

	SettingsItemInfo considerableUploadSpeed = SettingsItemInfo (tr ("Minimal up speed after which upload is considered active"), tr ("Network"), tr ("Performance"));
	considerableUploadSpeed.IntRange_ = qMakePair (256, 4096);
	considerableUploadSpeed.SpinboxSuffix_ = tr (" bytes/s");
	considerableUploadSpeed.SpinboxStep_ = 256;
	PropertyInfo_ ["ConsiderableUploadSpeed"] = considerableUploadSpeed;



	SettingsItemInfo rateControlTimerDelay = SettingsItemInfo (tr ("Rate control timer delay"), tr ("Interface"), tr ("Delays"));
	rateControlTimerDelay.SpinboxSuffix_ = tr (" ms");
	rateControlTimerDelay.SpinboxStep_ = 1000;
	rateControlTimerDelay.IntRange_ = qMakePair (0, 1200000);
	PropertyInfo_ ["RateControlTimerDelay"] = rateControlTimerDelay;

	SettingsItemInfo rateControlWindowLength = SettingsItemInfo (tr ("Rate control window length"), tr ("Interface"));
	rateControlWindowLength.IntRange_ = qMakePair (3, 15);
	PropertyInfo_ ["RateControlWindowLength"] = rateControlWindowLength;
}

SettingsManager* SettingsManager::Instance ()
{
	if (!Instance_)
		Instance_ = new SettingsManager;
	return Instance_;
}

void SettingsManager::Release ()
{
	Flush ();
	delete Instance_;
	Instance_ = 0;
}

void SettingsManager::Flush ()
{
	WriteSettings ();
}

int SettingsManager::GetPendingRequestTimeout () const
{
	return PendingRequestTimeout_;
}

void SettingsManager::SetPendingRequestTimeout (int val)
{
	PendingRequestTimeout_ = val;
}

int SettingsManager::GetClientTimeout () const
{
	return ClientTimeout_;
}

void SettingsManager::SetClientTimeout (int val)
{
	ClientTimeout_ = val;
}

int SettingsManager::GetConnectTimeout () const
{
	return ConnectTimeout_;
}

void SettingsManager::SetConnectTimeout (int val)
{
	ConnectTimeout_ = val;
}

int SettingsManager::GetKAInterval () const
{
	return KAInterval_;
}

void SettingsManager::SetKAInterval (int val)
{
	KAInterval_ = val;
}

int SettingsManager::GetRateControlTimerDelay () const
{
	return RateControlTimerDelay_;
}

void SettingsManager::SetRateControlTimerDelay (int val)
{
	RateControlTimerDelay_ = val;
}

int SettingsManager::GetServerMinPort () const
{
	return ServerMinPort_;
}

void SettingsManager::SetServerMinPort (int val)
{
	ServerMinPort_ = val;
}

int SettingsManager::GetServerMaxPort () const
{
	return ServerMaxPort_;
}

void SettingsManager::SetServerMaxPort (int val)
{
	ServerMaxPort_ = val;
}

int SettingsManager::GetBlockSize () const
{
	return BlockSize_;
}

void SettingsManager::SetBlockSize (int val)
{
	BlockSize_ = val;
}

int SettingsManager::GetMaxBlocksInProgress () const
{
	return MaxBlocksInProgress_;
}

void SettingsManager::SetMaxBlocksInProgress (int val)
{
	MaxBlocksInProgress_ = val;
}

int SettingsManager::GetMaxTotalConnections () const
{
	return MaxTotalConnections_;
}

void SettingsManager::SetMaxTotalConnections (int val)
{
	MaxTotalConnections_ = val;
}

int SettingsManager::GetRateControlWindowLength () const
{
	return RateControlWindowLength_;
}

void SettingsManager::SetRateControlWindowLength (int val)
{
	RateControlWindowLength_ = val;
}

int SettingsManager::GetUploadScheduleInterval () const
{
	return UploadScheduleInterval_;
}

void SettingsManager::SetUploadScheduleInterval (int val)
{
	UploadScheduleInterval_ = val;
}

int SettingsManager::GetEndGamePieces () const
{
	return EndGamePieces_;
}

void SettingsManager::SetEndGamePieces (int val)
{
	EndGamePieces_ = val;
}

int SettingsManager::GetMaxConnectionsPerPeer () const
{
	return MaxConnectionsPerPeer_;
}

void SettingsManager::SetMaxConnectionsPerPeer (int val)
{
	MaxConnectionsPerPeer_ = val;
}

int SettingsManager::GetMinimumRevisitTime () const
{
	return MinimumRevisitTime_;
}

void SettingsManager::SetMinimumRevisitTime (int val)
{
	MinimumRevisitTime_ = val;
}

int SettingsManager::GetMaxUploads () const
{
	return MaxUploads_;
}

void SettingsManager::SetMaxUploads (int val)
{
	MaxUploads_ = val;
}

int SettingsManager::GetMaxBlocksInMultiMode () const
{
	return MaxBlocksInMultiMode_;
}

void SettingsManager::SetMaxBlocksInMultiMode (int val)
{
	MaxBlocksInMultiMode_ = val;
}

int SettingsManager::GetConsiderableUploadSpeed () const
{
	return ConsiderableUploadSpeed_;
}

void SettingsManager::SetConsiderableUploadSpeed (int val)
{
	ConsiderableUploadSpeed_ = val;
}

/*
int SettingsManager::Get () const
{
	return _;
}

void SettingsManager::Set (int val)
{
	_ = val;
}
*/
SettingsItemInfo SettingsManager::GetInfoFor (const QString& name) const
{
	return PropertyInfo_ [name];
}

