CREATE TABLE components (
	component_id INTEGER PRIMARY KEY AUTOINCREMENT,
	repo_id INTEGER REFERENCES repos,
	component TEXT
);