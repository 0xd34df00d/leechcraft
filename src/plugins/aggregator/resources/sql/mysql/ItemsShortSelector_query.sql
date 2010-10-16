SELECT item_id, title, url, category, pub_date, unread 
    FROM items 
        WHERE channel_id = ? 
            ORDER BY unread ASC, pub_date DESC, title DESC

