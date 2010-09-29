CREATE TABLE mrss_comments (
    mrss_comment_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, 
    type TEXT, 
    comment TEXT
);

