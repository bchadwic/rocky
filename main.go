package main

import (
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
	ours.Push(outbound)
	for i := range locals {
		ours.Push(&locals[i])
	}

	theirs, err := app.Exchange(ours)
	if err != nil {
		return err
	}

	socket, err := net.ListenUDP("udp", &net.UDPAddr{IP: net.IPv4zero, Port: app.Port})
	if err != nil {
		return err
	}
	defer socket.Close()
	_ = socket.SetReadDeadline(time.Time{})

	for !theirs.Empty() {
		socket.WriteTo([]byte("hello"), theirs.Pop().Addr)
	}

	return nil
}
