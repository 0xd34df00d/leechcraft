#ifndef BENCODEPARSER_H
#define BENCODEPARSER_H
#include <QList>
#include <QMap>
#include <QVariant>
#include <QByteArray>
#include <QString>

typedef QMap<QByteArray, QVariant> Dictionary_t;
Q_DECLARE_METATYPE (Dictionary_t);

class BencodeParser
{
	QByteArray Code_;
	int Index_, InfoStart_, InfoLength_;

	Dictionary_t Parsed_;

	QString Error_;
public:
	bool Parse (const QByteArray& code);
	QString GetErrorString () const;
	const Dictionary_t& GetParsed () const;
	QByteArray GetInfoSection () const;
private:
	bool GetString (QByteArray&);
	bool GetInteger (qint64&);
	bool GetList (QList<QVariant>&);
	bool GetDictionary (Dictionary_t&);
};

#endif

