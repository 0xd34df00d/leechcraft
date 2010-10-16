SELECT mrss_comment_id, type, comment 
    FROM mrss_comments 
        WHERE mrss_id = ? 
            ORDER BY comment
