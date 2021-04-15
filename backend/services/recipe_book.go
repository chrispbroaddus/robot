package services

import (
	"github.com/satori/go.uuid"
	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/simulator"
)

// CreateRecipeBook creates a new recipe book from a series of new or old recipe steps
func CreateRecipeBook(store data.SimulatorRecipeBooks, book *simulator.CreateRecipeBook) (string, error) {
	for _, recipe := range book.Recipes {
		if recipe.ID == "" {
			recipe.ID = uuid.NewV4().String()
		}
	}

	dataBook := &data.SimulatorRecipeBook{
		ID:      uuid.NewV4().String(),
		Name:    book.Name,
		Version: book.Version,
		Recipes: book.Recipes,
	}

	err := store.CreateRecipeBook(dataBook)
	if err != nil {
		return "", err
	}

	return dataBook.ID, nil
}

// DeleteRecipeBook removes a recipe book from the storage
func DeleteRecipeBook(store data.SimulatorRecipeBooks, bookID string) error {
	return store.RemoveRecipeBook(bookID)
}

// RemoveRecipeStep deletes a recipe step from a recipe book
func RemoveRecipeStep(store data.SimulatorRecipeBooks, bookID, recipeID string) error {
	return store.RemoveRecipe(bookID, recipeID)
}

// AddNewRecipeStep writes a new step into data and assign it to a book
func AddNewRecipeStep(store data.SimulatorRecipeBooks, newRecipe *simulator.NewRecipeStep) error {
	if newRecipe.Recipe.ID == "" {
		newRecipe.Recipe.ID = uuid.NewV4().String()
	}

	return store.AddRecipe(newRecipe.RecipeBookID, newRecipe.Version, newRecipe.Recipe)
}

// FindRecipeBook gets a recipe book from the data store
func FindRecipeBook(store data.SimulatorRecipeBooks, bookID string) (*simulator.RecipeBookView, error) {
	dataBook, err := store.FindRecipeBook(bookID)
	if err != nil {
		return nil, err
	}

	return &simulator.RecipeBookView{
		ID:          dataBook.ID,
		Name:        dataBook.Name,
		Version:     dataBook.Version,
		RecipeSteps: dataBook.Recipes,
	}, nil
}
