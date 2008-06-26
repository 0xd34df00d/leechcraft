#ifndef REGEXPMATCHERMANAGER_H
#define REGEXPMATCHERMANAGER_H
#include <QAbstractItemModel>
#include <QStringList>
#include <deque>
#include <stdexcept>

class RegexpMatcherManager : public QAbstractItemModel
{
	Q_OBJECT
public:
	class AlreadyExists : public std::runtime_error
	{
	public:
		explicit AlreadyExists (const std::string& str)
		: std::runtime_error (str)
		{
		}
	};

	struct Item
	{
		QString Title_;
		QString Body_;

		Item (const QString& title, const QString& body)
		: Title_ (title)
		, Body_ (body)
		{
		}

		bool operator== (const Item& other) const
		{
			return Title_ == other.Title_ &&
				Body_ == other.Body_;
		}

		bool IsEqual (const QString& str) const
		{
			return Title_ == str;
		}
	};
private:

	QStringList ItemHeaders_;
	typedef std::deque<Item> items_t;
	items_t Items_;

	RegexpMatcherManager ();
public:
	static RegexpMatcherManager& Instance ();
	virtual ~RegexpMatcherManager ();

	void Add (const QString&, const QString&);

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
};

#endif

