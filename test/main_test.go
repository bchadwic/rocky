package main

import (
	"fmt"
	"os"
	"testing"

	"github.com/testcontainers/testcontainers-go/modules/compose"
)

// not thread safe
var stack *compose.DockerCompose

func TestMain(m *testing.M) {
	var err error
	stack, err = construct()
	if err != nil {
		fmt.Fprintf(os.Stderr, "could not construct docker compose environment: %v", err)
		os.Exit(1)
	}

	code := m.Run()

	teardown(stack)
	os.Exit(code)
}
