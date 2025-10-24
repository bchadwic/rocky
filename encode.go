package main

import (
	"encoding/base64"
	"encoding/binary"
	"fmt"
	"strings"
)

func encode(candidates *AddrCandidates) string {
	var encoded strings.Builder

	fmt.Println("encoding:")
	for !candidates.Empty() {
		candidate := candidates.Pop()
		addr := candidate.addr

		ipv4 := addr.IP.To4()
		if ipv4 == nil {
			continue
		}

		raw := make([]uint8, 6)
		copy(raw[0:4], ipv4)
		binary.BigEndian.PutUint16(raw[4:], uint16(addr.Port))

		encoded.WriteString(base64.StdEncoding.EncodeToString(raw))
		encoded.WriteByte(':')

		fmt.Printf("%v ", addr)
	}
	fmt.Println()

	n := encoded.Len()
	if n == 0 {
		return ""
	}

	return encoded.String()[:n-1]
}
