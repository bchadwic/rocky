package main

import (
	"fmt"
	"log"
)

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
		if err = candidates.Push(&local); err != nil {
			log.Printf("skipping %v: %v", &local, err)
		}
	}
	log.Printf("collected %d candidates", candidates.Len())

	encoded := encode(candidates)

	fmt.Println(encoded)
	return nil
}
