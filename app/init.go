package app

import (
	"context"
	"fmt"
	"log"
	"net"
	"time"
)

func TryConnect(ctx context.Context, cancel context.CancelFunc, src, dst *net.UDPAddr) {
	conn, err := net.DialUDP("udp4", src, dst)
	if err != nil {
		fmt.Printf("ERROR dialing: %v\n", err)
		return
	}
	defer conn.Close()

	delay := time.NewTicker(3 * time.Second)
	for {
		select {
		case <-ctx.Done():
			return
		default:
			for range 3 {
				_, err := conn.Write([]byte("hello"))
				if err != nil {
					fmt.Printf("ERROR writing: %v\n", err)
				}
			}
		}

		log.Printf("%v->%v\n", dst, conn.LocalAddr())

		select {
		case <-ctx.Done():
			return
		case <-delay.C:
		}
	}
}
