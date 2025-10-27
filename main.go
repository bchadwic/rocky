package main

import (
	"fmt"
	"log"
	"net"
	"time"

	"github.com/bchadwic/rocky/app"
)

const (
	ROCKYPort      = 40000
	BurstCount     = 5
	RapidInterval  = 50 * time.Millisecond
	RetryInterval  = 300 * time.Millisecond
	ConnectTimeout = 10 * time.Second
	ReadBufSize    = 1500
)

func burst(conn *net.UDPConn, peer *net.UDPAddr) error {
	for i := 0; i < BurstCount; i++ {
		_, err := conn.WriteToUDP([]byte("ping"), peer)
		if err != nil {
			return err
		}
		log.Printf("packet %d sent to %v", i+1, peer)
		time.Sleep(RapidInterval)
	}
	return nil
}

func tryConnect(conn *net.UDPConn, peer *net.UDPAddr) (*net.UDPAddr, error) {
	buf := make([]byte, ReadBufSize)
	deadline := time.Now().Add(ConnectTimeout)
	for {
		if err := burst(conn, peer); err != nil {
			return nil, err
		}

		conn.SetReadDeadline(time.Now().Add(100 * time.Millisecond))
		n, addr, err := conn.ReadFromUDP(buf)
		if err == nil && n > 0 {
			log.Printf("received packet from %v: %s", addr, string(buf[:n]))
			return addr, nil
		}

		if time.Now().After(deadline) {
			return nil, fmt.Errorf("timeout waiting for peer")
		}

		time.Sleep(RetryInterval)
	}
}

func holePunch() error {
	// bind to fixed port like C code
	local := &net.UDPAddr{IP: net.IPv4zero, Port: ROCKYPort}
	conn, err := net.ListenUDP("udp", local)
	if err != nil {
		return err
	}
	defer conn.Close()
	log.Printf("listening on %v", conn.LocalAddr())

	// get reflexive/outbound address
	reflexive, outbound, err := app.GetServerReflexiveAddress()
	if err != nil {
		return err
	}
	log.Printf("reflexive=%v outbound=%v", reflexive, outbound)

	// exchange candidates
	ours := app.NewAddrCandidates()
	ours.Push(reflexive)
	theirs, err := app.Exchange(ours)
	if err != nil {
		return err
	}
	peer := theirs.Pop().Addr

	// first tryConnect to update actual peer address
	actualPeer, err := tryConnect(conn, peer)
	if err != nil {
		return err
	}

	// connect socket to lock peer (like C connect)
	conn2, err := net.DialUDP("udp", conn.LocalAddr().(*net.UDPAddr), actualPeer)

	if err != nil {
		// if err := conn.ConnectUDP(actualPeer); err != nil {
		return err
	}
	log.Printf("connected to peer %v", actualPeer)

	// now you can send/receive with Write/Read directly
	_, err = conn2.Write([]byte("hello"))
	if err != nil {
		return err
	}

	buf := make([]byte, ReadBufSize)
	n, err := conn2.Read(buf)
	if err != nil {
		return err
	}
	fmt.Printf("reply received: %s\n", buf[:n])
	return nil
}

func main() {
	if err := holePunch(); err != nil {
		log.Fatalf("holepunch failed: %v", err)
	}
}

// func main() {
// 	err := run()
// 	if err != nil {
// 		log.Fatalln(err.Error())
// 	}
// }

// func run() error {
// 	reflexive, outbound, err := app.GetServerReflexiveAddress()
// 	if err != nil {
// 		return err
// 	}

// 	// locals, err := app.GetLocalAddresses()
// 	// if err != nil {
// 	// 	return err
// 	// }

// 	ours := app.NewAddrCandidates()
// 	ours.Push(reflexive)
// 	// ours.Push(outbound)
// 	// for i := range locals {
// 	// 	ours.Push(&locals[i])
// 	// }
// 	fmt.Printf("RFLX:%v, OUT: %v\n", reflexive, outbound)

// 	theirs, err := app.Exchange(ours)
// 	if err != nil {
// 		return err
// 	}

// 	// addr := &net.UDPAddr{IP: net.IPv4zero, Port: app.Port}
// 	outbound.Addr.Port = app.Port
// 	conn, err := net.ListenUDP("udp", outbound.Addr)
// 	if err != nil {
// 		return err
// 	}
// 	defer conn.Close()
// 	_ = conn.SetReadDeadline(time.Time{})

// 	ctx, cancel := context.WithCancel(context.Background())
// 	defer cancel()

// 	their := theirs.Pop().Addr
// 	go func() {
// 		delay := time.NewTicker(100 * time.Millisecond)
// 		for {
// 			select {
// 			case <-ctx.Done():
// 				return
// 			default:
// 				_, err = conn.WriteToUDP([]byte("hello"), their)
// 				if err != nil {
// 					log.Printf("%v\n", err)
// 					return
// 				}
// 				fmt.Printf("sending packet to %v\n", their)
// 			}

// 			select {
// 			case <-ctx.Done():
// 				return
// 			case <-delay.C:
// 			}
// 		}
// 	}()

// 	buf := make([]byte, 512)
// 	n, err := conn.Read(buf)
// 	if err != nil {
// 		return err
// 	}
// 	cancel()

// 	fmt.Printf("reply received: %s\n", buf[:n])
// 	return nil
// }
