package app

import (
	"context"
	"log"
	"net"
	"time"
)

func TryConnect(ctx context.Context, cancel context.CancelFunc, src, dst *net.UDPAddr) {
	conn, err := net.DialUDP("udp4", src, dst)
	if err != nil {
		log.Printf("could not dial %v\n", err)
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
					log.Printf("could not dial %v\n", err)
				}
			}
		}

		log.Printf("sent 'hello' to %v from %v\n", dst, conn.LocalAddr())

		select {
		case <-ctx.Done():
			return
		case <-delay.C:
		}
	}
}
