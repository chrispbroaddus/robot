package simulator

import (
	"github.com/zippyai/zippy/backend/data"
)

// CreateRecipeBook is used to crete a new recipe book used for starting a simulator cluster
type CreateRecipeBook struct {
	Name    string                  `json:"name"`
	Version string                  `json:"version"`
	Recipes []*data.SimulatorRecipe `json:"recipe_steps"`
}

// NewRecipePayload payload that is returned when a new recipe book is created
type NewRecipePayload struct {
	ID      string `json:"id"`
	Name    string `json:"name"`
	Version string `json:"version"`
}

// NewRecipeStep payload to create a new recipe step for a given recipe book
type NewRecipeStep struct {
	RecipeBookID string                `json:"book_id"`
	Version      string                `json:"new_version"`
	Recipe       *data.SimulatorRecipe `json:"recipe"`
}

// RecipeStepPayload payload returned when a new recipe step has been created
type RecipeStepPayload struct {
	RecipeID string `json:"id"`
	Version  string `json:"version"`
}

// DeleteRecipeStep payload of the recipe id to delete and the book to delete it from
type DeleteRecipeStep struct {
	RecipeBookID string `json:"book_id"`
	RecipeID     string `json:"recipe_id"`
}

// RecipeBookView is the entire view of a recipe book
type RecipeBookView struct {
	ID          string                  `json:"id"`
	Name        string                  `json:"name"`
	Version     string                  `json:"version"`
	RecipeSteps []*data.SimulatorRecipe `json:"recipe_steps"`
}
