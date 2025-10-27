package main

import (
	"context"
	"fmt"
	"log"
	"net"
	"time"

	"github.com/bchadwic/rocky/app"
)

const (
	burstCount     = 5
	burstInterval  = 100 * time.Millisecond
	retryInterval  = 300 * time.Millisecond
	connectTimeout = 10 * time.Second
	readBufSize    = 1500
)

func burstSend(conn *net.UDPConn, peer *net.UDPAddr) error {
	for i := 0; i < burstCount; i++ {
		_, err := conn.WriteToUDP([]byte("ping"), peer)
		if err != nil {
			return err
		}
		log.Printf("sent burst packet %d -> %s", i+1, peer)
		time.Sleep(burstInterval)
	}
	return nil
}

func holePunchTest() error {
	// get reflexive and the local outbound addr used by STUN
	reflexive, outbound, err := app.GetServerReflexiveAddress()
	if err != nil {
		return err
	}
	fmt.Printf("reflexive=%v outbound=%v\n", reflexive, outbound)

	// exchange candidates with remote (assume app.Exchange exists)
	ours := app.NewAddrCandidates()
	ours.Push(reflexive)
	// ours.Push(outbound)

	theirs, err := app.Exchange(ours)
	if err != nil {
		return err
	}
	peer := theirs.Pop().Addr

	// bind to the same local address/port that STUN used to create the mapping
	// outbound.Addr must contain the local IP and port used for STUN
	local := outbound.Addr
	conn, err := net.ListenUDP("udp", local)
	if err != nil {
		return err
	}
	defer conn.Close()
	log.Printf("listening on %s (bound to preserve NAT mapping)", conn.LocalAddr())

	// allow quick reads without blocking forever
	deadlinePerRead := 1 * time.Second
	ctx, cancel := context.WithTimeout(context.Background(), connectTimeout)
	defer cancel()

	// sender goroutine: repeat bursts until we receive or timeout
	recvCh := make(chan []byte, 1)
	errCh := make(chan error, 1)

	go func() {
		buf := make([]byte, readBufSize)
		for {
			_ = conn.SetReadDeadline(time.Now().Add(deadlinePerRead))
			n, addr, err := conn.ReadFromUDP(buf)
			if err != nil {
				// timeout or other; return to allow retrying
				if ne, ok := err.(net.Error); ok && ne.Timeout() {
					select {
					case <-ctx.Done():
						errCh <- ctx.Err()
						return
					default:
						// continue waiting
						continue
					}
				}
				errCh <- err
				return
			}
			log.Printf("received %d bytes from %s", n, addr)
			// send one-shot back to signal success then exit
			recvCh <- append([]byte(nil), buf[:n]...)
			return
		}
	}()

	// actively punch: bursts then sleep and repeat until read or timeout
	ticker := time.NewTicker(retryInterval)
	defer ticker.Stop()

	for {
		if err := burstSend(conn, peer); err != nil {
			return err
		}

		select {
		case <-ctx.Done():
			return fmt.Errorf("timeout waiting for peer reply: %w", ctx.Err())
		case b := <-recvCh:
			fmt.Printf("reply received: %s\n", string(b))
			return nil
		case err := <-errCh:
			return err
		case <-ticker.C:
			// retry burst
			continue
		}
	}
}

func main() {
	if err := holePunchTest(); err != nil {
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
