package data

import (
	"database/sql"
	"errors"
	"fmt"
	"time"
)

const (
	baseSimDockerCmd = "sudo nvidia-docker run --rm -d -e \"DISPLAY=:0\""
	baseDockerCmd    = "sudo docker run --rm -d"
)

var (
	// ErrNoRecipeBook is returned if there is no recipe book for a given id
	ErrNoRecipeBook = errors.New("there is no recipe book for the provided id")
	// ErrNoRecipe is returned if there is no recipe record for a given id
	ErrNoRecipe = errors.New("there is no recipe for the provided id")
)

// ResourceAlloc is a list of the hardware reqs needed to run a session and is used as a ticket for the RA
type ResourceAlloc struct {
	// DeploymentReqs maps an id for a simulatorRecipe to the resource req for it
	DeploymentReqs map[string]*ContainerReq
	SessionID      string
	RecipeBookID   string
}

// ContainerReq requirements for a specific container
type ContainerReq struct {
	CPU    float64 `json:"cpu"`
	Memory float64 `json:"memory"`
	GPU    int     `json:"gpu"`
}

// SimulatorRecipeBook stores a full sim deployment info
type SimulatorRecipeBook struct {
	ID      string
	Name    string
	Version string
	Recipes []*SimulatorRecipe
}

// SimulatorRecipe is the individual information for each container that makes up the sim
type SimulatorRecipe struct {
	ID      string          `json:"recipe_id,omitempty"`
	Image   string          `json:"image"`
	Tag     string          `json:"tag"`
	Count   int             `json:"container_count"`
	Req     *ContainerReq   `json:"hardware_req"`
	Volumes []*DockerVolume `json:"volumes"`
	Ports   []*PortMap      `json:"ports"`
	Command string          `json:"run_command"`
}

// DockerVolume is the information on the volumes a container uses
type DockerVolume struct {
	Source      string `json:"source"`
	Destination string `json:"dest"`
}

// PortMap is the information for a containers port mappings
type PortMap struct {
	Container int `json:"container"`
	Host      int `json:"host"`
}

// CreateRecipeBook is the PersistentStorage implementation to write a recipe book to storage
func (p *PersistentStorage) CreateRecipeBook(r *SimulatorRecipeBook) error {
	defer func(now time.Time) {
		requestTime.With("method", createRecipeBook, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	tx, err := p.db.Begin()
	if err != nil {
		return err
	}
	defer recoverRollBack(tx)

	stmt, err := tx.Prepare(`INSERT INTO recipe_books(id, name, version) VALUES($1,$2,$3);`)
	if err != nil {
		return err
	}
	defer stmt.Close()

	if _, err := stmt.Exec(r.ID, r.Name, r.Version); err != nil {
		tx.Rollback()
		return err
	}

	for _, recipe := range r.Recipes {
		if err := p.insertRecipeRecords(tx, recipe, r.ID); err != nil {
			tx.Rollback()
			return err
		}
	}

	return tx.Commit()
}

// RemoveRecipeBook is the PersistentStorage implementation to delete a recipe book from the db
func (p *PersistentStorage) RemoveRecipeBook(recipeBookID string) error {
	defer func(now time.Time) {
		requestTime.With("method", removeRecipeBook, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	tx, err := p.db.Begin()
	if err != nil {
		return err
	}
	defer recoverRollBack(tx)

	// delete the linking recipe_book recipe record
	{
		stmt, err := tx.Prepare(`DELETE FROM recipe_book_recipes WHERE recipe_book_id = $1;`)
		if err != nil {
			tx.Rollback()
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(recipeBookID); err != nil {
			tx.Rollback()
			return err
		}
	}

	// delete the recipe_book record
	{
		stmt, err := tx.Prepare(`DELETE FROM recipe_books WHERE id = $1;`)
		if err != nil {
			tx.Rollback()
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(recipeBookID); err != nil {
			tx.Rollback()
			return err
		}
	}
	return tx.Commit()
}

// RemoveRecipe is the PersistentStorage implementation to write a recipe book to storage
func (p *PersistentStorage) RemoveRecipe(recipeBookID, recipeID string) error {
	defer func(now time.Time) {
		requestTime.With("method", removeRecipe, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	tx, err := p.db.Begin()
	if err != nil {
		return err
	}
	defer recoverRollBack(tx)

	// delete recipe link to recipe_book
	{
		stmt, err := tx.Prepare(`DELETE FROM recipe_book_recipes WHERE recipe_id = $1;`)
		if err != nil {
			tx.Rollback()
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(recipeID); err != nil {
			tx.Rollback()
			return err
		}
	}

	// delete recipe volumes
	{
		stmt, err := tx.Prepare(`DELETE FROM recipe_volumes WHERE recipe_id = $1;`)
		if err != nil {
			tx.Rollback()
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(recipeID); err != nil {
			tx.Rollback()
			return err
		}
	}

	// delete recipe ports
	{
		stmt, err := tx.Prepare(`DELETE FROM recipe_ports WHERE recipe_id = $1;`)
		if err != nil {
			tx.Rollback()
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(recipeID); err != nil {
			tx.Rollback()
			return err
		}
	}

	// delete recipe record
	{
		stmt, err := tx.Prepare(`DELETE FROM recipes WHERE id = $1;`)
		if err != nil {
			tx.Rollback()
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(recipeID); err != nil {
			tx.Rollback()
			return err
		}
	}

	return tx.Commit()
}

// AddRecipe is the PersistentStorage implementation to write a recipe book to storage
func (p *PersistentStorage) AddRecipe(recipeBookID, version string, r *SimulatorRecipe) error {
	defer func(now time.Time) {
		requestTime.With("method", addRecipe, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	tx, err := p.db.Begin()
	if err != nil {
		return err
	}
	defer recoverRollBack(tx)

	// insert all of the recipe values
	if err := p.insertRecipeRecords(tx, r, recipeBookID); err != nil {
		tx.Rollback()
		return err
	}

	// update recipe book version
	{
		stmt, err := tx.Prepare(`UPDATE recipe_books SET version = $1 WHERE id = $2;`)
		if err != nil {
			tx.Rollback()
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(version, recipeBookID); err != nil {
			tx.Rollback()
			return err
		}
	}

	return tx.Commit()
}

// FindRecipeBook is the PersistentStorage implementation to pull a recipe book from storage
func (p *PersistentStorage) FindRecipeBook(bookID string) (*SimulatorRecipeBook, error) {
	defer func(now time.Time) {
		requestTime.With("method", findRecipeBook, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	stmt, err := p.db.Prepare(`SELECT book.*, recipe.id, recipe.image, recipe.tag, recipe.container_count, recipe.command, recipe.cpu_req, recipe.mem_req, recipe.gpu_req, recipe.source, recipe.destination, recipe.container, recipe.host 
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
                        ON rb.recipe_id = r.id) AS recipe 
		ON recipe.recipe_book_id = $1
	WHERE book.id = $1;`)
	if err != nil {
		return nil, err
	}

	rows, err := stmt.Query(bookID)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	book := new(SimulatorRecipeBook)
	var currentRecipe *SimulatorRecipe

	for rows.Next() {
		recipe := new(SimulatorRecipe)
		volume := new(DockerVolume)
		port := new(PortMap)
		req := new(ContainerReq)

		err = rows.Scan(&book.ID, &book.Name, &book.Version,
			&recipe.ID, &recipe.Image, &recipe.Tag, &recipe.Count, &recipe.Command,
			&req.CPU, &req.Memory, &req.GPU,
			&volume.Source, &volume.Destination,
			&port.Container, &port.Host)
		if err != nil {
			return nil, err
		}

		// this should only happen on the first run through just to give an initial value to currentRecipe
		if currentRecipe == nil {
			currentRecipe = recipe
		}

		// add the current recipe to list of recipes when we got to a new recipe row
		if currentRecipe.ID != recipe.ID {
			book.Recipes = append(book.Recipes, currentRecipe)
			currentRecipe = recipe
		}

		currentRecipe.Req = req

		// do not add empty volumes
		if volume != nil && volume.Destination != "" && volume.Source != "" {
			currentRecipe.Volumes = append(currentRecipe.Volumes, volume)
		}

		// do not add empty port mappings
		if port != nil && (port.Container > 0 || port.Host > 0) {
			currentRecipe.Ports = append(currentRecipe.Ports, port)
		}
	}

	// add the last
	book.Recipes = append(book.Recipes, currentRecipe)

	if err := rows.Err(); err != nil {
		return nil, err
	}

	return book, nil
}

func (p *PersistentStorage) insertRecipeRecords(tx *sql.Tx, r *SimulatorRecipe, bookID string) error {
	defer recoverRollBack(tx)

	// create a new recipe record
	{
		stmt, err := tx.Prepare(`INSERT INTO recipes(id, image, tag, container_count, cpu_req, mem_req, gpu_req, command) 
	VALUES($1,$2,$3,$4,$5,$6,$7,$8);`)
		if err != nil {
			return err
		}
		defer stmt.Close()

		_, err = stmt.Exec(r.ID, r.Image, r.Tag, r.Count, r.Req.CPU, r.Req.Memory, r.Req.GPU, r.Command)
		if err != nil {
			return err
		}
	}

	// link the new recipe to the recipe_book that uses it
	{
		stmt, err := tx.Prepare(`INSERT INTO recipe_book_recipes(recipe_book_id, recipe_id) VALUES($1,$2);`)
		if err != nil {
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(bookID, r.ID); err != nil {
			return err
		}
	}

	// create all of the volume records for a recipe
	for _, volume := range r.Volumes {
		stmt, err := tx.Prepare(`INSERT INTO recipe_volumes(recipe_id, source, destination) VALUES($1,$2,$3);`)
		if err != nil {
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(r.ID, volume.Source, volume.Destination); err != nil {
			return err
		}
	}

	// create all of the port records for a recipe
	for _, port := range r.Ports {
		stmt, err := tx.Prepare(`INSERT INTO recipe_ports(recipe_id, container, host) VALUES($1,$2,$3);`)
		if err != nil {
			return err
		}
		defer stmt.Close()

		if _, err := stmt.Exec(r.ID, port.Container, port.Host); err != nil {
			return err
		}
	}

	return nil
}

func recoverRollBack(tx *sql.Tx) {
	if r := recover(); r != nil {
		tx.Rollback()
		panic(r)
	}
}

// CreateRecipeBook is the localSimHostStorage implementation to write a recipe book to storage
func (l *localSimHostStorage) CreateRecipeBook(r *SimulatorRecipeBook) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	l.recipeBooks[r.ID] = r

	for _, recipe := range r.Recipes {
		l.recipes[recipe.ID] = recipe
	}

	return nil
}

// RemoveRecipeBook is the localSimHostStorage implementation to delete a recipe book from the db
func (l *localSimHostStorage) RemoveRecipeBook(recipeBookID string) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	delete(l.recipeBooks, recipeBookID)

	return nil
}

// RemoveRecipe is the localSimHostStorage implementation to write a recipe book to storage
func (l *localSimHostStorage) RemoveRecipe(recipeBookID, recipeID string) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	delete(l.recipes, recipeID)

	book, ok := l.recipeBooks[recipeBookID]
	if !ok {
		return ErrNoRecipeBook
	}

	for i, recipe := range book.Recipes {
		if recipe.ID != recipeID {
			continue
		}

		// remove the recipe record from the book
		book.Recipes = append(book.Recipes[:i], book.Recipes[i+1:]...)

		return nil
	}

	return ErrNoRecipe
}

// AddRecipe is the localSimHostStorage implementation to write a recipe book to storage
func (l *localSimHostStorage) AddRecipe(recipeBookID, version string, r *SimulatorRecipe) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	l.recipes[r.ID] = r

	book, ok := l.recipeBooks[recipeBookID]
	if !ok {
		return ErrNoRecipeBook
	}

	book.Recipes = append(book.Recipes, r)

	return nil
}

// FindRecipeBook is the localSimHostStorage implementation to pull a recipe book from storage
func (l *localSimHostStorage) FindRecipeBook(bookID string) (*SimulatorRecipeBook, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	book, ok := l.recipeBooks[bookID]
	if !ok {
		return nil, ErrNoRecipeBook
	}

	return book, nil
}

// BuildResourceReq create a ResourceAlloc from a SimulatorRecipeBook
func (r *SimulatorRecipeBook) BuildResourceReq(sessionID string) *ResourceAlloc {
	alloc := &ResourceAlloc{
		SessionID:      sessionID,
		RecipeBookID:   r.ID,
		DeploymentReqs: make(map[string]*ContainerReq),
	}

	for _, req := range r.Recipes {
		alloc.DeploymentReqs[req.ID] = req.Req
	}

	return alloc
}

// GatherPlacementCommands gets every recipe steps docker commands for placing the instances
func (r *SimulatorRecipeBook) GatherPlacementCommands() map[string][]string {
	result := make(map[string][]string)

	for _, recipe := range r.Recipes {
		result[recipe.ID] = recipe.createDockerCommands()
	}

	return result
}

// CreateDockerCommands makes all of the commands needed to run a particular recipe
func (r *SimulatorRecipe) createDockerCommands() []string {
	resultCommands := make([]string, r.Count)

	// if we need a gpu compute resource that means we're running a simulator and need to use the nvidia-docker wrapper
	cmd := baseDockerCmd
	if r.Req.GPU > 0 {
		cmd = baseSimDockerCmd
	}

	for _, volume := range r.Volumes {
		cmd = fmt.Sprintf("%s --volume=%s:%s", cmd, volume.Source, volume.Destination)
	}

	for _, port := range r.Ports {
		cmd = fmt.Sprintf("%s --port=%d:%d", cmd, port.Host, port.Container)
	}

	// set the image to use in the command
	cmd = fmt.Sprintf("%s %s:%s", cmd, r.Image, r.Tag)

	// add the entry point command
	cmd = fmt.Sprintf("%s %s", cmd, r.Command)

	for i := 0; i < r.Count; i++ {
		resultCommands[i] = cmd
	}

	return resultCommands
}
