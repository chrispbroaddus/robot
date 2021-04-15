package simulatornode

import (
	"encoding/json"
	"log"
	"net/http"

	"github.com/gorilla/mux"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/simulator"
)

func (s *SessionManager) createRecipeBook(w http.ResponseWriter, r *http.Request) {
	pathTmpl, err := mux.CurrentRoute(r).GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	newBook := new(simulator.CreateRecipeBook)
	if err := json.NewDecoder(r.Body).Decode(newBook); err != nil {
		log.Println("unable to decode new recipe book for creation", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	bookID, err := services.CreateRecipeBook(s.storage, newBook)
	if err != nil {
		log.Println("unable to create a new recipe book in storage: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	respondJSON(w, r, "", &simulator.NewRecipePayload{
		ID:      bookID,
		Name:    newBook.Name,
		Version: newBook.Version,
	})
}

func (s *SessionManager) deleteRecipeBook(w http.ResponseWriter, r *http.Request) {
	recipeBookID := mux.Vars(r)["book_id"]

	pathTmpl, err := mux.CurrentRoute(r).GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	if err := services.DeleteRecipeBook(s.storage, recipeBookID); err != nil {
		log.Printf("unable to delete the recipe book with id: %s in storage: %s", recipeBookID, err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	w.WriteHeader(http.StatusOK)
}

func (s *SessionManager) addRecipeStep(w http.ResponseWriter, r *http.Request) {
	pathTmpl, err := mux.CurrentRoute(r).GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	recipe := new(simulator.NewRecipeStep)
	if err := json.NewDecoder(r.Body).Decode(recipe); err != nil {
		log.Println("unable to decode new recipe step for creation", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	if err := services.AddNewRecipeStep(s.storage, recipe); err != nil {
		log.Printf("unable to write new step for recipe book with id: %s error: %s\n", recipe.RecipeBookID, err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	respondJSON(w, r, "", &simulator.RecipeStepPayload{
		RecipeID: recipe.Recipe.ID,
		Version:  recipe.Version,
	})
}

func (s *SessionManager) deleteRecipeStep(w http.ResponseWriter, r *http.Request) {
	pathTmpl, err := mux.CurrentRoute(r).GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	deleteRecipe := new(simulator.DeleteRecipeStep)
	if err := json.NewDecoder(r.Body).Decode(deleteRecipe); err != nil {
		log.Println("unable to decode recipe to delete", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	if err := services.RemoveRecipeStep(s.storage, deleteRecipe.RecipeBookID, deleteRecipe.RecipeID); err != nil {
		log.Printf("unable to delete recipe with id: %s step for recipe book with id: %s error: %s\n", deleteRecipe.RecipeID, deleteRecipe.RecipeBookID, err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	w.WriteHeader(http.StatusOK)
}

func (s *SessionManager) viewRecipeBook(w http.ResponseWriter, r *http.Request) {
	recipeBookID := mux.Vars(r)["book_id"]

	pathTmpl, err := mux.CurrentRoute(r).GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	recipeBook, err := services.FindRecipeBook(s.storage, recipeBookID)
	if err != nil {
		log.Printf("unable to find recipe book with id: %s error: %s\n", recipeBookID, err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	respondJSON(w, r, "", recipeBook)
}
