CREATE TABLE recipe_books (
    id UUID PRIMARY KEY,
    name VARCHAR(75) NOT NULL,
    version VARCHAR(150) NOT NULL
);

CREATE TABLE recipes (
    id UUID PRIMARY KEY,
    image VARCHAR(250) NOT NULL,
    tag VARCHAR(75) DEFAULT 'latest',
    container_count INTEGER DEFAULT 1,
    cpu_req DECIMAL(8, 4),
    mem_req DECIMAL(8, 4),
    gpu_req INTEGER,
    command VARCHAR(250)
);

CREATE TABLE recipe_book_recipes (
    recipe_book_id UUID REFERENCES recipe_books(id),
    recipe_id UUID REFERENCES recipes(id)
);

CREATE TABLE recipe_volumes (
    recipe_id UUID REFERENCES recipes(id),
    source VARCHAR(250) NOT NULL,
    destination VARCHAR(250) NOT NULL
);

CREATE TABLE recipe_ports (
    recipe_id UUID REFERENCES recipes(id),
    container INTEGER,
    host INTEGER
);