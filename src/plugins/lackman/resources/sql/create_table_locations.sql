CREATE TABLE locations (
	package_id INTEGER REFERENCES packages ON DELETE CASCADE,
	component_id INTEGER REFERENCES components
);