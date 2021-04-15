package teleopui

import (
	"testing"

	"github.com/stretchr/testify/require"
)

func TestAssetExists(t *testing.T) {
	// This test just checks that the assets required by appication exist at the expected paths
	_, err := Asset("index.html")
	require.NoError(t, err)
}
