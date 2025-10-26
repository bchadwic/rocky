package app

import (
	"context"
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
				_, _ = socket.WriteToUDP([]byte("hello"), addr)
			}
		}

		select {
		case <-ctx.Done():
			return
		case <-delay.C:
		}
	}
}
