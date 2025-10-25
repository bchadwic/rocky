package main

import (
	"fmt"
	"log"
	"time"
)

func animate(stop chan bool) {
	frames := []string{"-", "\\", "|", "/"}
	i := 0
	for {
		select {
		case <-stop:
			fmt.Printf("\x1b[2K\r")
			return
		default:
			fmt.Printf("\rLoading %s", frames[i%len(frames)])
			i++
			time.Sleep(100 * time.Millisecond)
		}
	}
}

func main() {
	err := run()
	if err != nil {
		log.Fatalln(err.Error())
	}
}

func run() error {
	candidates := NewAddrCandidates()

	// STUN
	reflexive, outbound, err := getServerReflexiveAddress()
	if err != nil {
		return err
	}
	candidates.Push(reflexive)
	candidates.Push(outbound)

	// Local Interfaces
	locals, err := getLocalAddresses()
	if err != nil {
		return err
	}

	for i := range locals {
		local := locals[i]
		_ = candidates.Push(&local)
	}

	encoded := encode(candidates)
	fmt.Printf("%s\n   \\___ peer exchange: ", encoded)

	var in string
	fmt.Scanf("%s", &in)

	stop := make(chan bool)
	go animate(stop)

	time.Sleep(3 * time.Second)
	stop <- true

	return nil
}
