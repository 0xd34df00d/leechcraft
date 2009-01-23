#ifndef IFINDER_H
#define IFINDER_H
#include <QStringList>
#include <QMap>

namespace LeechCraft
{
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

// emits entityUpdated(FoundEntity) in case of new/updated entity.
class IFinder
{
public:
	virtual ~IFinder () {}

	virtual QStringList GetCategories () const = 0;
	virtual void Start (const QString&, const QStringList&, bool) = 0;
	virtual void Abort () = 0;
};

Q_DECLARE_INTERFACE (IFinder, "org.Deviant.LeechCraft.IFinder/1.0");

#endif

