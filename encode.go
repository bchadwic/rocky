package main

import (
	"encoding/base64"
	"encoding/binary"
	"strings"
)

func encode(candidates *AddrCandidates) string {
	var encoded strings.Builder

	for !candidates.Empty() {
		candidate := candidates.Pop()
		addr := candidate.addr

		ip := addr.IP.To4()
		if ip == nil {
			continue
		}

		raw := make([]uint8, 4)
		copy(raw, ip)
		if addr.Port != 0 {
			raw = binary.BigEndian.AppendUint16(raw, uint16(addr.Port))
		}

		encoded.WriteString(base64.RawURLEncoding.EncodeToString(raw))
		encoded.WriteByte(':')
	}

	n := encoded.Len()
	if n == 0 {
		return ""
	}

	return encoded.String()[:n-1]
}
