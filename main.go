package main

import (
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

	// Local Interfaces
	locals, err := getLocalAddresses()
	if err != nil {
		return err
	}

	for i := range locals {
		candidates.Push(&locals[i])
	}
	candidates.Push(reflexive)
	candidates.Push(outbound)

	candidates, err = exchange(candidates)
	if err != nil {
		return err
	}

	return nil
}
