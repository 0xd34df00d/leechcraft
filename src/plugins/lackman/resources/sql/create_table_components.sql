CREATE TABLE components (
	component_id INTEGER PRIMARY KEY AUTOINCREMENT,
	repo_id INTEGER REFERENCES repos CHECK (repo_id >= 0),
	component TEXT
);