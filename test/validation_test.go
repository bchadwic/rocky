package main

import (
	"context"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func Test_Validation(t *testing.T) {
	ctx := context.Background()
	peer_a1, err := stack.ServiceContainer(ctx, "peer_a1")
	assert.NoError(t, err)

	tcases := map[string]struct {
		cmd    string
		code   int
		stderr string
	}{
		"no args": {
			cmd:    "odon",
			code:   1,
			stderr: "command not provided\n",
		},
		"only one arg send": {
			cmd:    "odon send",
			code:   1,
			stderr: "send needs 1 arg\n",
		},
		"only one arg recv": {
			cmd:    "odon recv",
			code:   1,
			stderr: "recv needs 1 arg\n",
		},
		"not a valid file": {
			cmd:    "odon send non-existent.txt",
			code:   1,
			stderr: "run: No such file or directory\n",
		},
	}

	for tname, tcase := range tcases {
		t.Run(tname, func(t *testing.T) {
			code, output, err := peer_a1.Exec(ctx, []string{"sh", "-c", tcase.cmd})
			assert.NoError(t, err)

			_, stderr := parse(output)
			require.Equal(t, tcase.code, code)
			require.Equal(t, tcase.stderr, stderr.String())
		})
	}
}
