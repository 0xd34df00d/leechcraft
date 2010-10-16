SELECT enclosure_id, url, type, length, lang 
    FROM enclosures 
        WHERE item_id = ? 
            ORDER BY url
