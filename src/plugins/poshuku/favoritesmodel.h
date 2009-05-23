#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H
#include <vector>
#include <QAbstractItemModel>
#include <QStringList>

class FavoritesModel : public QAbstractItemModel
{
	Q_OBJECT
	
	QStringList ItemHeaders_;
public:
	struct FavoritesItem
	{
		QString Title_;
		QString URL_;
		/// Contains ids of the real tags.
		QStringList Tags_;

		bool operator== (const FavoritesItem&) const;
	};
	typedef std::vector<FavoritesItem> items_t;
private:
	items_t Items_;
public:
	enum Columns
	{
		ColumnTitle
		, ColumnURL
		, ColumnTags
	};

	enum
	{
		/// Returns the user-sensible string with tags.
		TagsRole = 42
	};

	FavoritesModel (QObject* = 0);
	virtual ~FavoritesModel ();

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation,
			int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int,
			const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
	virtual bool setData (const QModelIndex&, const QVariant&,
			int = Qt::EditRole);

	bool AddItem (const QString&, const QString&, const QStringList&);
	const items_t& GetItems () const;
private:
	QStringList GetVisibleTags (int) const;
public slots:
	void removeItem (const QModelIndex&);
	void handleItemAdded (const FavoritesModel::FavoritesItem&);
	void handleItemUpdated (const FavoritesModel::FavoritesItem&);
	void handleItemRemoved (const FavoritesModel::FavoritesItem&);
private slots:
	void loadData ();
signals:
	void error (const QString&);
};

#endif

