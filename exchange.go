package main

import (
	"encoding/base64"
	"encoding/binary"
	"fmt"
	"net"
	"strings"
)

func exchange(candidates *AddrCandidates) (*AddrCandidates, error) {
	ourCandidates, err := serialize(candidates)
	if err != nil {
		return nil, err
	}

	var theirCandidates string
	fmt.Printf("%s\n   \\__ peer exchange: ", ourCandidates)
	fmt.Scanf("%s", &theirCandidates)

	return deserialize(theirCandidates)
}

func serialize(candidates *AddrCandidates) (string, error) {
	var encoded strings.Builder

	for !candidates.Empty() {
		candidate := candidates.Pop()
		addr := candidate.addr

		ip := addr.IP.To4()
		if ip == nil {
			continue
		}

		raw := []byte(ip)
		if addr.Port != 0 {
			raw = binary.BigEndian.AppendUint16(raw, uint16(addr.Port))
		}

		encoded.WriteString(base64.RawURLEncoding.EncodeToString(raw))
		encoded.WriteByte(':')
	}

	n := encoded.Len()
	if n == 0 {
		return "", fmt.Errorf("no active network address candidates available")
	}

	return encoded.String()[:n-1], nil
}

func deserialize(encoded string) (*AddrCandidates, error) {
	candidates := NewAddrCandidates()

	segments := strings.Split(encoded, ":")
	for priority, segment := range segments {

		addr := &net.UDPAddr{}

		s := []byte(segment)
		switch len(s) {
		case 6: // local network address
			b := make([]byte, 4)
			if n, err := base64.RawURLEncoding.Decode(b, s); err != nil || n != 4 {
				continue
			}

			addr.IP = net.IP(b[:4])
		case 8: // reflexive network address
			b := make([]byte, 6)
			if n, err := base64.RawURLEncoding.Decode(b, s); err != nil || n != 6 {
				continue
			}

			addr.IP = net.IP(b[:4])
			addr.Port = int(binary.BigEndian.Uint16(b[4:6]))
		default:
			continue
		}

		candidate := &AddrCandidate{priority: priority, addr: addr}
		candidates.Push(candidate)
	}

	if candidates.Empty() {
		return nil, fmt.Errorf("peer had no valid candidates")
	}

	return candidates, nil
}
