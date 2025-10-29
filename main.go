package main

import (
	"fmt"
	"log"
	"net"
	"strconv"
	"syscall"
	"time"

	"github.com/pion/stun"
)

func main() {
	err := run()
	if err != nil {
		log.Fatalln(err.Error())
	}
	err = test()
	if err != nil {
		log.Fatalln(err.Error())
	}
}

func test() error {
	// Parse a STUN URI
	u, err := stun.ParseURI("stun:stun.l.google.com:19302")
	if err != nil {
		panic(err)
	}

	// Creating a "connection" to STUN server.
	c, err := stun.DialURI(u, &stun.DialConfig{})
	if err != nil {
		panic(err)
	}
	// Building binding request with random transaction id.
	message := stun.MustBuild(stun.TransactionID, stun.BindingRequest)
	// Sending request to STUN server, waiting for response message.
	if err := c.Do(message, func(res stun.Event) {
		if res.Error != nil {
			panic(res.Error)
		}
		// Decoding XOR-MAPPED-ADDRESS attribute from message.
		var xorAddr stun.XORMappedAddress
		if err := xorAddr.GetFrom(res.Message); err != nil {
			panic(err)
		}

		fmt.Printf("public: %v:%d\n", xorAddr.IP, xorAddr.Port)
	}); err != nil {
		panic(err)
	}
	return nil
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
	ip4 := ip.To4() // converts to 4-byte form
	if ip4 == nil {
		return fmt.Errorf("not an ipv4 address")
	}
	peer := &syscall.SockaddrInet4{Addr: [4]byte{ip4[0], ip4[1], ip4[2], ip4[3]}, Port: _port}
	fmt.Println("received addr:", peer.Addr)

	fd, err := syscall.Socket(syscall.AF_INET, syscall.SOCK_DGRAM, syscall.IPPROTO_UDP)
	if err != nil {
		return err
	}
	defer syscall.Close(fd)

	err = syscall.Bind(fd, &syscall.SockaddrInet4{
		Port: 60357,
		Addr: [4]byte(net.IPv4zero),
	})
	if err != nil {
		return err
	}

	tv := syscall.Timeval{Sec: 3, Usec: 0}
	err = syscall.SetsockoptTimeval(fd, syscall.SOL_SOCKET, syscall.SO_RCVTIMEO, &tv)
	if err != nil {
		return err
	}

	tryConnect(fd, peer)

	fmt.Println("connected!")
	return nil
}

func tryConnect(fd int, peerAddr *syscall.SockaddrInet4) error {
	var actualPeer syscall.SockaddrInet4
	buf := make([]byte, 512)

	for {
		if err := burst(fd, peerAddr); err != nil {
			return err
		}

		n, from, err := syscall.Recvfrom(fd, buf, 0)
		if err != nil {
			if err != syscall.EAGAIN && err != syscall.EWOULDBLOCK {
				return err
			}
		} else {
			switch sa := from.(type) {
			case *syscall.SockaddrInet4:
				actualPeer = *sa
			case *syscall.SockaddrInet6:
				// handle IPv6 if needed
				return fmt.Errorf("unexpected IPv6 address")
			default:
				return fmt.Errorf("unexpected address family")
			}

			fmt.Printf("received packet from peer with contents: %.*s\n", n, buf[:n])
			break
		}

		fmt.Println("could not connect, retrying")
		time.Sleep(200 * time.Millisecond)
	}

	peerAddr.Addr = actualPeer.Addr
	peerAddr.Port = actualPeer.Port
	return syscall.Connect(fd, peerAddr)
}

func burst(fd int, peer *syscall.SockaddrInet4) error {
	for i := 1; i <= 3; i++ {
		if err := syscall.Sendto(fd, []byte("ping"), 0, peer); err != nil {
			return err
		}

		fmt.Printf("packet %d sent\n", i)
		time.Sleep(10 * time.Millisecond)
	}
	return nil

}
