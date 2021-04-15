BEGIN;

INSERT INTO recipe_books(id, name, version) VALUES('c33524a3-0c77-4485-8569-d149bc394f58', 'test_book', '0.0.1');

INSERT INTO recipes(id, image, tag, container_count, cpu_req, mem_req, gpu_req, command)
    VALUES('6a116fe3-c1dd-4936-bfd6-ab224a4f3670', 'teleop', 'master', 1, 110.5, 110.5, 1, 'run.sh');

INSERT INTO recipe_book_recipes(recipe_book_id, recipe_id) VALUES('c33524a3-0c77-4485-8569-d149bc394f58', '6a116fe3-c1dd-4936-bfd6-ab224a4f3670');

INSERT INTO recipe_volumes(recipe_id, source, destination) VALUES('6a116fe3-c1dd-4936-bfd6-ab224a4f3670', '/bin/bash', '/bash');

INSERT INTO recipes(id, image, tag, container_count, cpu_req, mem_req, gpu_req, command)
    VALUES('2a8045c7-57d9-474d-a071-36681fd9934e', 'simmy', 'master', 1, 110.5, 110.5, 1, 'run.sh');

INSERT INTO recipe_book_recipes(recipe_book_id, recipe_id) VALUES('c33524a3-0c77-4485-8569-d149bc394f58', '2a8045c7-57d9-474d-a071-36681fd9934e');

INSERT INTO recipe_ports(recipe_id, container) VALUES('2a8045c7-57d9-474d-a071-36681fd9934e', 8080);

COMMIT;