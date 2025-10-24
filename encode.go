package main

import (
	"encoding/base64"
	"encoding/binary"
	"fmt"
	"net"
	"strings"
)

func encode(addrs []net.UDPAddr) string {
	var encoded strings.Builder

	for _, addr := range addrs {
		raw := make([]uint8, 6)
		copy(raw, addr.IP)
		binary.BigEndian.PutUint16(raw[4:], uint16(addr.Port))

		fmt.Println(raw)
		encoded.WriteString(base64.StdEncoding.EncodeToString(raw))
		encoded.WriteByte(':')
	}

	n := encoded.Len()
	if n == 0 {
		return ""
	}

	return encoded.String()[:n-1]
}
