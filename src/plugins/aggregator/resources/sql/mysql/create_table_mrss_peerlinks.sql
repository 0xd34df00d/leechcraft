CREATE TABLE mrss_peerlinks (
    mrss_peerlink_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, 
    type TEXT, 
    link TEXT
);

