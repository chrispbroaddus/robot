package simulatornode

import (
	"encoding/json"
	"fmt"
	"net/http"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/simulator"
)

var (
	newTestBook = &simulator.CreateRecipeBook{
		Name:    "test_book",
		Version: "0.0.1",
		Recipes: []*data.SimulatorRecipe{
			&data.SimulatorRecipe{
				ID:    "dd078b8c-fbe9-4d0d-a5d4-5e2ccad055ff",
				Image: "test_image",
				Tag:   "master",
				Count: 1,
				Req: &data.ContainerReq{
					CPU:    1,
					Memory: 100,
					GPU:    1,
				},
				Command: "run.sh",
			},
		},
	}
	newRecipeStep = &simulator.NewRecipeStep{
		Version: "0.0.2",
		Recipe: &data.SimulatorRecipe{
			Image: "test_image",
			Tag:   "master",
			Count: 1,
			Req: &data.ContainerReq{
				CPU:    1,
				Memory: 100,
				GPU:    1,
			},
			Command: "run.sh",
		},
	}
)

func TestRecipeBook_CreateBook(t *testing.T) {
	req, err := makeReq(server.URL, "/recipe/book/create", "POST", newTestBook, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	respPayload := new(simulator.NewRecipePayload)
	err = json.NewDecoder(resp.Body).Decode(respPayload)
	require.NoError(t, err)
	assert.Equal(t, newTestBook.Name, respPayload.Name)
	assert.Equal(t, newTestBook.Version, respPayload.Version)
}

func TestRecipeBook_DeleteBook(t *testing.T) {
	// create a recipe book first
	req, err := makeReq(server.URL, "/recipe/book/create", "POST", newTestBook, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	respPayload := new(simulator.NewRecipePayload)
	err = json.NewDecoder(resp.Body).Decode(respPayload)
	require.NoError(t, err)
	assert.Equal(t, newTestBook.Name, respPayload.Name)
	assert.Equal(t, newTestBook.Version, respPayload.Version)

	req, err = makeReq(server.URL, fmt.Sprintf("/recipe/book/%s/delete", respPayload.ID), "DELETE", nil, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()
}

func TestRecipeBook_AddRecipeStep(t *testing.T) {
	// create a recipe book first
	req, err := makeReq(server.URL, "/recipe/book/create", "POST", newTestBook, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	respPayload := new(simulator.NewRecipePayload)
	err = json.NewDecoder(resp.Body).Decode(respPayload)
	require.NoError(t, err)
	assert.Equal(t, newTestBook.Name, respPayload.Name)
	assert.Equal(t, newTestBook.Version, respPayload.Version)

	newRecipeStep.RecipeBookID = respPayload.ID
	req, err = makeReq(server.URL, "/recipe/add", "POST", newRecipeStep, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()
	// reset the id so we don't mess up future tests
	newRecipeStep.RecipeBookID = ""

	newRecipePayload := new(simulator.RecipeStepPayload)
	err = json.NewDecoder(resp.Body).Decode(newRecipePayload)
	require.NoError(t, err)
	assert.Equal(t, newRecipeStep.Version, newRecipePayload.Version)
}

func TestRecipeBook_DeleteRecipeStep(t *testing.T) {
	// create a recipe book first
	req, err := makeReq(server.URL, "/recipe/book/create", "POST", newTestBook, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	respPayload := new(simulator.NewRecipePayload)
	err = json.NewDecoder(resp.Body).Decode(respPayload)
	require.NoError(t, err)
	assert.Equal(t, newTestBook.Name, respPayload.Name)
	assert.Equal(t, newTestBook.Version, respPayload.Version)

	deleteRecipe := &simulator.DeleteRecipeStep{
		RecipeBookID: respPayload.ID,
		RecipeID:     newTestBook.Recipes[0].ID,
	}

	req, err = makeReq(server.URL, "/recipe/delete", "DELETE", deleteRecipe, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()
}

func TestRecipeBook_ViewRecipeBook(t *testing.T) {
	// create a recipe book first
	req, err := makeReq(server.URL, "/recipe/book/create", "POST", newTestBook, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	respPayload := new(simulator.NewRecipePayload)
	err = json.NewDecoder(resp.Body).Decode(respPayload)
	require.NoError(t, err)
	assert.Equal(t, newTestBook.Name, respPayload.Name)
	assert.Equal(t, newTestBook.Version, respPayload.Version)

	req, err = makeReq(server.URL, fmt.Sprintf("/recipe/book/%s", respPayload.ID), "GET", nil, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	bookView := new(simulator.RecipeBookView)
	err = json.NewDecoder(resp.Body).Decode(bookView)
	require.NoError(t, err)
	assert.Equal(t, respPayload.Name, bookView.Name)
	assert.Equal(t, respPayload.Version, bookView.Version)
	assert.Len(t, bookView.RecipeSteps, len(newTestBook.Recipes))
}
