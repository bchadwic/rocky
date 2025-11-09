package main

import (
	"fmt"
	"os"
	"os/exec"
)

func main() {
	if err := run(); err != nil {
		fmt.Printf("run error: %v\n", err)
		os.Exit(1)
	}
}

func run() error {
	if err := instantiate(); err != nil {
		return err
	}
	defer func() {
		if err := teardown(); err != nil {
			fmt.Printf("teardown error: %v\n", err)
		}
	}()
	return nil
}

func instantiate() error {
	cmd := exec.Command("docker", "compose", "up", "--detach", "--remove-orphans")
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

func teardown() error {
	cmd := exec.Command("docker", "compose", "down", "--rmi", "all", "--volumes", "--remove-orphans")
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

// 	ctx := context.Background()

// 	cli, err := client.NewClientWithOpts(client.WithAPIVersionNegotiation())
// 	if err != nil {
// 		return err
// 	}

// 	networks := map[string]string{}
// 	containers := map[string]string{}

// 	for _, name := range []string{"internet", "network-a", "network-b"} {
// 		id, err := createNetwork(ctx, cli, name)
// 		if err != nil {
// 			return err
// 		}
// 		defer deleteNetwork(ctx, cli, id)
// 		networks[name] = id
// 	}

// 	for _, name := range []string{"peer-a", "peer-b"} {
// 		id, err := createPeer(ctx, cli, name)
// 		if err != nil {
// 			return err
// 		}
// 		defer deletePeer(ctx, cli, id)
// 		containers[name] = id
// 	}

// 	err = cli.NetworkConnect(ctx, networks["network-a"], containers["peer-a"], &network.EndpointSettings{})
// 	if err != nil {
// 		return err
// 	}

// 	return nil
// }

// func createNetwork(ctx context.Context, cli *client.Client, name string) (string, error) {
// 	opts := network.CreateOptions{
// 		Driver:     "bridge",
// 		Scope:      "local",
// 		EnableIPv4: func(b bool) *bool { return &b }(true),
// 	}
// 	resp, err := cli.NetworkCreate(ctx, name, opts)
// 	if err != nil {
// 		return "", err
// 	}
// 	return resp.ID, nil
// }

// func deleteNetwork(ctx context.Context, cli *client.Client, id string) error {
// 	return cli.NetworkRemove(ctx, id)
// }

// func createPeer(ctx context.Context, cli *client.Client, name string) (string, error) {
// 	config := &container.Config{
// 		Image: "alpine",
// 		Cmd:   []string{"sh"},
// 	}

// 	resp, err := cli.ContainerCreate(ctx, config, nil, nil, nil, name)
// 	if err != nil {
// 		return "", err
// 	}

// 	return resp.ID, nil
// }

// func deletePeer(ctx context.Context, cli *client.Client, id string) error {
// 	stopOpts := container.StopOptions{
// 		Timeout: func(i int) *int { return &i }(0),
// 	}
// 	if err := cli.ContainerStop(ctx, id, stopOpts); err != nil {
// 		return err
// 	}

// 	rmOpts := container.RemoveOptions{
// 		Force: true,
// 	}
// 	return cli.ContainerRemove(ctx, id, rmOpts)
// }
