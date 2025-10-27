package main

import (
	"context"
	"fmt"
	"log"
	"net"
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
	reflexive, _, err := app.GetServerReflexiveAddress()
	if err != nil {
		return err
	}

	// locals, err := app.GetLocalAddresses()
	// if err != nil {
	// 	return err
	// }

	ours := app.NewAddrCandidates()
	ours.Push(reflexive)
	// ours.Push(outbound)
	// for i := range locals {
	// 	ours.Push(&locals[i])
	// }
	// fmt.Printf("RFLX:%v, OUT: %v\n", reflexive, outbound)

	theirs, err := app.Exchange(ours)
	if err != nil {
		return err
	}

	addr := &net.UDPAddr{IP: net.IPv4zero, Port: app.Port}
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		return err
	}
	defer conn.Close()
	_ = conn.SetReadDeadline(time.Time{})

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	go func() {
		delay := time.NewTicker(100 * time.Millisecond)
		for {
			select {
			case <-ctx.Done():
				return
			default:
				_, err = conn.WriteToUDP([]byte("hello"), theirs.Pop().Addr)
				if err != nil {
					log.Printf("%v\n", err)
					return
				}
			}

			select {
			case <-ctx.Done():
				return
			case <-delay.C:
			}
		}
	}()

	buf := make([]byte, 512)
	n, err := conn.Read(buf)
	if err != nil {
		return err
	}

	fmt.Printf("reply received: %s\n", buf[:n])
	return nil
}
