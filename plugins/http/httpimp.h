#ifndef HTTPIMP_H
#define HTTPIMP_H
#include <QMap>
#include <QPair>
#include <plugininterface/guarded.h>
#include "impbase.h"

class TcpSocket;
class Proxy;
class QMutex;
class QWaitCondition;

class HttpImp : public ImpBase
{
	Q_OBJECT
public:
	enum ResponseCode
	{
		Continue			= 100,
		SwitchingProtocols	= 101,
		Processing_WEBDAV_	= 102,

		OK					= 200,
		Created				= 201,
		Accepted			= 202,
		NonAuthoritative	= 203,
		NoContent			= 204,
		ResetContent		= 205,
		PartialContent		= 206,
		MultiStatus_WEBDAV_	= 207,

		MultipleChoices		= 300,
		MovedPermanently	= 301,
		Found				= 302,
		SeeOther			= 303,
		NotModified			= 304,
		UseProxy			= 305,
		SwitchProxy			= 306,
		TemporaryRedirect	= 307,

		BadRequest						= 400,
		Unauthorized					= 401,
		PaymentRequired					= 402,
		Forbidden						= 403,
		NotFound						= 404,
		MethodNotAllowed				= 405,
		NotAcceptable					= 406,
		ProxyAuthenticationRequired		= 407,
		RequestTimeout					= 408,
		Conflict						= 409,
		Gone							= 410,
		LengthRequired					= 411,
		PreconditionFailed				= 412,
		RequestEntityTooLarge			= 413,
		RequestURITooLong				= 414,
		UnsupportedMediaType			= 415,
		RequestedRangeNotSatisfiable	= 416,
		ExpectationFailed				= 417,
		UnprocessableEntity_WEBDAV_		= 422,
		Locked_WEBDAV_					= 423,
		FailedDependency_WEBDAV_		= 424,
		UnorderedCollection_WEBDAV_		= 425,
		UpgradeRequired					= 426,
		RetryWith						= 449,

		InternalServerError			= 500,
		NotImplemented				= 501,
		BadGateway					= 502,
		ServiceUnavailable			= 503,
		GatewayTimeout				= 504,
		HTTPVersionNotSupported		= 505,
		VariantAlsoNegotiates		= 506,
		InsufficientStorage_WEBDAV_	= 507,
		BandwidthLimitExceeded		= 509,
		NotExtented					= 510
	};
private:
	struct Response
	{
		QString Proto_;
		int StatusCode_;
		QString StatusReason_;
		QMap<QString, QString> Fields_;
		length_t ContentLength_;
	};
	Response Response_;
	TcpSocket *Socket_;

	Guarded<bool> Stop_, GetFileSize_;
	QPair<QWaitCondition*, QMutex*> AwaitFileInfoReaction_;
public:
	HttpImp (QObject *parent = 0);
	virtual ~HttpImp ();
	virtual void SetRestartPosition (length_t);
	virtual void SetURL (const QString&);
	virtual void StopDownload ();
	virtual void ReactedToFileInfo ();
	virtual void ScheduleGetFileSize ();
private:
	virtual void run ();
	void WriteHeaders ();
	bool ReadResponse ();
	void ParseFirstLine (const QString&);
	bool DoPrimaryStuffWithResponse ();
	bool DoSecondaryStuffWithResponse ();
	void ParseSingleLine (const QString&);
	void DoRedirect ();
	void Finalize ();
signals:
	void clarifyURL (QString);
};

#endif

