package main

import (
	"bytes"
	"context"
	"fmt"
	"io"

	"github.com/docker/docker/pkg/stdcopy"
	"github.com/testcontainers/testcontainers-go/modules/compose"
)

func construct() (*compose.DockerCompose, error) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	stack, err := compose.NewDockerComposeWith(
		compose.WithStackFiles("docker-compose.yaml"),
	)
	if err != nil {
		return nil, fmt.Errorf("an error occurred starting docker compose file: %w", err)
	}

	if err = stack.Up(ctx, compose.Wait(true)); err != nil {
		return nil, fmt.Errorf("an error occurred waiting for docker compose resources: %w", err)
	}
	return stack, nil
}

func teardown(stack *compose.DockerCompose) {
	err := stack.Down(
		context.Background(),
		compose.RemoveOrphans(true),
		compose.RemoveVolumes(true),
		compose.RemoveImagesLocal,
	)
	if err != nil {
		fmt.Printf("an error occurred shutting down and removing docker compose resources: %v", err)
	}
}

func parse(output io.Reader) (stdout, stderr bytes.Buffer) {
	stdcopy.StdCopy(&stdout, &stderr, output)
	return stdout, stderr
}
