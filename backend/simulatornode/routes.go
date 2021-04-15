package simulatornode

import (
	"github.com/gorilla/mux"
)

// NewSimulatorHostRouter sets up the api routes for the simulator host
func (s *SessionManager) NewSimulatorHostRouter() *mux.Router {
	r := mux.NewRouter()

	// start a new simulator
	r.HandleFunc("/simulator/start", s.startSimulator).Methods("POST")

	// stop a currently running simulator
	r.HandleFunc("/simulator/{sim_id}/stop", s.stopSimulator).Methods("DELETE")

	// recipe book related endpoints
	// create new recipe book
	r.HandleFunc("/recipe/book/create", s.createRecipeBook).Methods("POST")

	// delete recipe book
	r.HandleFunc("/recipe/book/{book_id}/delete", s.deleteRecipeBook).Methods("DELETE")

	// add a recipe step from a book
	r.HandleFunc("/recipe/add", s.addRecipeStep).Methods("POST")

	// delete a recipe step from a book
	r.HandleFunc("/recipe/delete", s.deleteRecipeStep).Methods("DELETE")

	// return all of the recipe book information from an id
	r.HandleFunc("/recipe/book/{book_id}", s.viewRecipeBook).Methods("GET")

	return r
}
