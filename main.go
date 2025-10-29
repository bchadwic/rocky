package main

import (
	"fmt"
	"log"
	"net"
	"strconv"
	"time"
)

func main() {
	err := run()
	if err != nil {
		log.Fatalln(err.Error())
	}
}

func run() error {
	reflexive, _, err := getServerReflexiveAddress()
	if err != nil {
		return err
	}

	fmt.Println("public:", reflexive.addr)
	var in string
	fmt.Print("peer: ")
	fmt.Scanf("%s", &in)

	host, port, err := net.SplitHostPort(in)
	if err != nil {
		return err
	}

	_port, _ := strconv.Atoi(port)
	ip := net.ParseIP(host)
	ipv4 := ip.To4() // converts to 4-byte form
	if ipv4 == nil {
		return fmt.Errorf("not an ipv4 address")
	}
	peer := &net.UDPAddr{IP: ipv4, Port: _port}
	local := &net.UDPAddr{IP: net.IPv4zero, Port: AppPort}
	conn, err := net.DialUDP("udp4", local, peer)
	if err != nil {
		return err
	}
	defer conn.Close()

	for {
		reply, err := Send(conn, "hello")
		if err != nil {
			fmt.Println("error", err)
		} else {
			fmt.Println("info", string(reply))
		}
		time.Sleep(800 * time.Millisecond)
	}

	return nil
}

func Send(conn *net.UDPConn, msg string) ([]byte, error) {
	_, err := conn.Write([]byte(msg))
	if err != nil {
		return nil, err
	}
	conn.SetReadDeadline(time.Now().Add(500 * time.Millisecond))
	buf := make([]byte, 512)
	_, err = conn.Read(buf)
	if err != nil {
		return nil, err
	}
	return buf, nil
}
