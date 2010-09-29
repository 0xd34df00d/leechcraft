UPDATE items 
    SET description = ? , author = ? , category = ? , pub_date = ? , unread = ? , 
        num_comments = ? , comments_url = ? , comments_page_url = ? , latitude = ? , longitude = ? 
            WHERE item_id = ? 
