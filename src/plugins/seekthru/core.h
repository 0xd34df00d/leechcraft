#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <QStringList>

struct UrlDescription
{
	QString Template_;
	QString Type_;
	int IndexOffset_;
	int PageOffset_;
};

struct QueryDescription
{
	enum Role
	{
		RoleRequest,
		RoleExample,
		RoleRelated,
		RoleCorrection,
		RoleSubset,
		RoleSuperset
	};

	Role Role_;
	QString Title_;
	int TotalResults_;
	QString SearchTerms_;
	int Count_;
	int StartIndex_;
	int StartPage_;
	QString Language_;
	QString InputEncoding_;
	QString OutputEncoding_;
};

struct Description
{
	enum SyndicationRight
	{
		SROpen,
		SRLimited,
		SRPrivate,
		SRClosed
	};

	QString ShortName_;
	QString Description_;
	QList<UrlDescription> URLs_;
	QString Contact_;
	QStringList Tags_;
	QString LongName_;
	QList<QueryDescription> Queries_;
	QString Developer_;
	QString Attribution_;
	SyndicationRight Right_;
	bool Adult_;
	QStringList Languages_;
	QStringList InputEncodings_;
	QStringList OutputEncodings_;
};

class Core : public QObject
{
	Q_OBJECT

	Core ();
public:
	static Core& Instance ();
	void Add (const QString&);
signals:
	void error (const QString&);
};

#endif

