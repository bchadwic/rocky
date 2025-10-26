package app

import (
	"context"
	"log"
	"net"
	"time"
)

func TryConnect(ctx context.Context, cancel context.CancelFunc, socket *net.UDPConn, addr *net.UDPAddr) {
	delay := time.NewTicker(3 * time.Second)
	for {
		select {
		case <-ctx.Done():
			return
		default:
			for range 3 {
				_, err := socket.WriteToUDP([]byte("hello"), addr)
				if err != nil {
					log.Println(err.Error())
				}
			}
		}

		select {
		case <-ctx.Done():
			return
		case <-delay.C:
		}
		log.Printf("sent 'hello' to %v\n", addr)
	}
}
