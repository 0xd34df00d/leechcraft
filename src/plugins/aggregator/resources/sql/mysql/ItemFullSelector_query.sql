SELECT title, url, description, author, category, guid, pub_date, unread, num_comments, 
    comments_url, comments_page_url, latitude, longitude, channel_id, item_id
    FROM items 
        WHERE item_id = ? 
            ORDER BY pub_date DESC

