package app

import (
	"context"
	"fmt"
	"net"
	"time"
)

func TryConnect(ctx context.Context, cancel context.CancelFunc, src, dst *net.UDPAddr) {
	src.Port = 0
	conn, err := net.ListenUDP("udp4", src)
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
				_, err := conn.WriteToUDP([]byte("hello"), dst)
				if err != nil {
					fmt.Printf("ERROR writing: %v\n", err)
				}
			}
		}

		fmt.Printf("%v->%v\n", dst, conn.LocalAddr())

		select {
		case <-ctx.Done():
			return
		case <-delay.C:
		}
	}
}
