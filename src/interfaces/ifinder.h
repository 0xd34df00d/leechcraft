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
};

class IFindProxy
{
public:
	enum
	{
		/** The role for the widget appearing on the right part of the
		 * screen when the user selects an item.
		 */
		RoleWidget = 50,
		/** The role for the hash of the item, used to compare two
		 * different results, possibly from two different models.
		 */
		RoleHash
	};
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

