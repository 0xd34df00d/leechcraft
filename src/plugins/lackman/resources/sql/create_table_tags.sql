CREATE TABLE tags (
	package_id INTEGER REFERENCES packages,
	tag TEXT NOT NULL
);