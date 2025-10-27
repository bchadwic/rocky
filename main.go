package main

import (
	"context"
	"fmt"
	"log"
	"net"
	"sync"
	"time"

	"github.com/bchadwic/rocky/app"
)

func main() {
	err := run()
	if err != nil {
		log.Fatalln(err.Error())
	}
}

func run() error {
	reflexive, outbound, err := app.GetServerReflexiveAddress()
	if err != nil {
		return err
	}

	locals, err := app.GetLocalAddresses()
	if err != nil {
		return err
	}

	ours := app.NewAddrCandidates()
	ours.Push(reflexive)
	// ours.Push(outbound)
	// for i := range locals {
	// 	ours.Push(&locals[i])
	// }
	log.Printf("reflexive: %v, outbound: %v", reflexive, outbound)

	theirs, err := app.Exchange(ours)
	if err != nil {
		return err
	}

	addr := &net.UDPAddr{IP: net.IPv4zero, Port: app.Port}
	socket, err := net.ListenUDP("udp4", addr)
	if err != nil {
		return err
	}
	defer socket.Close()
	_ = socket.SetReadDeadline(time.Time{})
	log.Printf("listening on %v\n", addr)

	var wg sync.WaitGroup
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	srcs := []net.UDPAddr{*reflexive.Addr, *outbound.Addr}
	for _, local := range locals {
		srcs = append(srcs, *local.Addr)
	}

	for !theirs.Empty() {
		dst := theirs.Pop().Addr
		for i := range srcs {
			src := &srcs[i]
			wg.Go(func() { app.TryConnect(ctx, cancel, src, dst) })
			time.Sleep(20 * time.Millisecond)
		}
	}

	log.Printf("reading from udp, waiting on replies\n")
	buf := make([]byte, 512)
	n, addr, err := socket.ReadFromUDP(buf)
	cancel()
	wg.Wait()

	if err != nil {
		return err
	}

	fmt.Printf("recieved: %s, from %v\n", string(buf[:n]), addr)

	return nil
}
