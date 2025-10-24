package main

import (
	"fmt"
	"log"
	"net"
)

func main() {
	err := run()
	if err != nil {
		log.Fatalln(err.Error())
	}
}

func run() error {
	conn, err := net.ListenUDP("udp", &net.UDPAddr{IP: net.IPv4zero, Port: 52838})
	if err != nil {
		return err
	}
	defer conn.Close()

	public, err := getpublicaddress(conn)
	if err != nil {
		return err
	}

	locals, err := getlocaladdresses()
	if err != nil {
		return err
	}

	candidates := append(locals, *public)
	encoded := encode(candidates)

	fmt.Println(encoded)
	return nil
}
