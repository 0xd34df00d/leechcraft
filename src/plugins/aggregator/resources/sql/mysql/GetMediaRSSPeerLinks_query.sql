SELECT mrss_peerlink_id, type, link 
    FROM mrss_peerlinks 
        WHERE mrss_id = ? 
            ORDER BY link
