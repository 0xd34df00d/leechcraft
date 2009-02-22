#ifndef IFINDER_H
#define IFINDER_H
#include <boost/shared_ptr.hpp>
#include <QStringList>
#include <QMap>

class QAbstractItemModel;

namespace LeechCraft
{
	struct Request
	{
		enum Type
		{
			RTFixed,
			RTWildcard,
			RTRegexp,
			RTTag
		};

		bool CaseSensitive_;
		Type Type_;
		QString Plugin_;
		QString Category_;
		QString String_;
		QStringList Params_;
	};

	struct FoundEntity
	{
		// As it could be handled by some plugin. URI, torrent file
		// contents, whatever.
		QByteArray Entity_;
		// Visible to user.
		QString Description_;
		// Category that the entity is related to, should be the same as in
		// GetCategories.
		QStringList Categories_;

		enum HashType
		{
			HTMD4,
			HTMD5,
			HTSHA1
		};

		// To allow comparisons between results of different search plugins.
		QMap<HashType, QByteArray> Hashes_;
	};
};

class IFindProxy
{
public:
	virtual ~IFindProxy () {}

	virtual QAbstractItemModel* GetModel () = 0;
};

class IFinder
{
public:
	virtual ~IFinder () {}

	virtual QStringList GetCategories () const = 0;
	virtual boost::shared_ptr<IFindProxy> GetProxy (const LeechCraft::Request&) = 0;
};

Q_DECLARE_INTERFACE (IFinder, "org.Deviant.LeechCraft.IFinder/1.0");
Q_DECLARE_INTERFACE (IFindProxy, "org.Deviant.LeechCraft.IFinder/1.0");

#endif

