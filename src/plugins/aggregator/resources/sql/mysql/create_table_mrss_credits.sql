CREATE TABLE mrss_credits (
    mrss_credits_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, 
    role TEXT, 
    who TEXT
);

