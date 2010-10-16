DELETE FROM items WHERE channel_id = ? AND DATE_ADD( pub_date, INTERVAL ? DAY ) < now();
