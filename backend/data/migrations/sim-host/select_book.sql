SELECT book.*, recipe.id, recipe.image, recipe.tag, recipe.container_count, recipe.command, recipe.cpu_req, recipe.mem_req, recipe.gpu_req, recipe.source, recipe.destination, recipe.container, recipe.host 
FROM 
    recipe_books AS book
    INNER JOIN (SELECT
                    r.*,
                    rb.recipe_book_id
                FROM
                    recipe_book_recipes AS rb
                    RIGHT JOIN (SELECT
                                    rr.*,
                                    COALESCE(v.source, '') AS source,
                                    COALESCE(v.destination, '') AS destination,
                                    COALESCE(p.container, 0) AS container,
                                    COALESCE(p.host, 0) AS host
                                FROM 
                                    recipes as rr
                                    LEFT JOIN recipe_volumes AS v
                                        ON rr.id = v.recipe_id
                                    LEFT JOIN recipe_ports AS p
                                        ON rr.id = p.recipe_id
                    ) AS r
                        ON rb.recipe_id = r.id
     ) AS recipe 
        ON recipe.recipe_book_id = 'c33524a3-0c77-4485-8569-d149bc394f58'
WHERE book.id = 'c33524a3-0c77-4485-8569-d149bc394f58';