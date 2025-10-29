package main

import (
	"crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"time"
)

func getServerReflexiveAddress() (*AddrCandidate, *AddrCandidate, error) {
	const (
		host = "stun.cloudflare.com"
		port = 3478
	)

	ips, err := net.LookupIP(host)
	if err != nil {
		return nil, nil, err
	}

	if len(ips) == 0 {
		return nil, nil, fmt.Errorf("could not resolve hostname: %s:%d", host, port)
	}

	try := func(conn *net.UDPConn) (*AddrCandidate, *AddrCandidate, error) {
		buf, err := pack()
		if err != nil {
			return nil, nil, err
		}

		if _, err := conn.Write(buf); err != nil {
			return nil, nil, err
		}

		resp := make([]byte, 512)
		conn.SetReadDeadline(time.Now().Add(800 * time.Millisecond))
		n, err := conn.Read(resp)
		if err != nil {
			return nil, nil, err
		}

		publicAddr, err := unpack(resp[:n])
		if err != nil {
			return nil, nil, err
		}

		sLocalIp, _, err := net.SplitHostPort(conn.LocalAddr().String())
		if err != nil {
			return nil, nil, fmt.Errorf("STUN client ip/port could not be parsed: %s - %v", conn.LocalAddr().String(), err)
		}

		localIp := net.ParseIP(sLocalIp)
		if localIp.To4() == nil {
			return nil, nil, fmt.Errorf("STUN request not sent over ipv4: %s", conn.LocalAddr().String())
		}

		return &AddrCandidate{
				priority: ServerReflexiveAddress,
				addr:     publicAddr,
			}, &AddrCandidate{
				priority: LocalOutboundAddress,
				addr:     &net.UDPAddr{IP: localIp, Port: 0}, // all local address ports should be hardcoded to zero
			}, nil
	}

	errs := []error{}
	for _, ip := range ips {
		if ip.To4() == nil {
			continue
		}

		attempt := &net.UDPAddr{IP: ip.To4(), Port: port}
		conn, err := net.DialUDP("udp", &net.UDPAddr{IP: net.IPv4zero, Port: 60357}, attempt)
		if err != nil {
			errs = append(errs, err)
			continue
		}

		local, public, err := try(conn)
		conn.Close()

		if err != nil {
			errs = append(errs, err)
			continue
		}
		return local, public, nil
	}

	return nil, nil, errors.Join(errs...)
}

func pack() ([]byte, error) {
	const (
		binding      uint16 = 0x0001
		noAttributes uint16 = 0x0000
		magicCookie  uint32 = 0x2112A442
	)

	buf := make([]byte, 20)
	binary.BigEndian.PutUint16(buf[0:], binding)
	binary.BigEndian.PutUint16(buf[2:], noAttributes)
	binary.BigEndian.PutUint32(buf[4:], magicCookie)
	if _, err := rand.Read(buf[8:]); err != nil { // random transaction id
		return nil, fmt.Errorf("could not create random transaction id for STUN message: %v", err)
	}
	return buf, nil
}

func unpack(buf []byte) (*net.UDPAddr, error) {
	const (
		headerLength            = 20
		binding          uint16 = 0x0101
		xorMappedAddress uint16 = 0x0020
		magicCookie      uint32 = 0x2112A442
		ipv4             uint8  = 0x01
	)
	ntohs := binary.BigEndian.Uint16
	ntohl := binary.BigEndian.Uint32

	if len(buf) < headerLength {
		return nil, fmt.Errorf("STUN reply less than 20 bytes: % X", buf)
	}

	pos := 0
	if typ := ntohs(buf[pos:]); typ != binding {
		return nil, fmt.Errorf("STUN reply type is not binding reply: % X", buf)
	}
	pos += 2

	if length := ntohs(buf[pos:]); int(length) > len(buf)-headerLength {
		return nil, fmt.Errorf("STUN reply attributes exceed buffer size: % X", buf)
	}
	pos += 2

	if cookie := ntohl(buf[pos:]); cookie != magicCookie {
		return nil, fmt.Errorf("STUN reply type magic cookie does not match: % X", buf)
	}
	pos += 4 + 12 // 4B cookie + 12B transaction id
	// TODO; verify transaction id

	for pos+4 <= len(buf) {
		typ := ntohs(buf[pos:])
		pos += 2
		length := ntohs(buf[pos:])
		pos += 2

		// skip any non xor mapped address attributes
		if typ != xorMappedAddress {
			pos += int(length)
			continue
		}

		if pos+8 > len(buf) {
			return nil, fmt.Errorf("STUN xor mapped address attribute value is improperly sized: % X", buf[pos:])
		}

		// skip first byte
		if buf[pos+1] != ipv4 {
			return nil, fmt.Errorf("STUN reply address family is unsupported: %02X", buf[pos])
		}
		pos += 2

		port := int(ntohs(buf[pos:]) ^ uint16(magicCookie>>16))
		pos += 2

		ip := make(net.IP, 4)
		binary.BigEndian.PutUint32(ip, ntohl(buf[pos:])^magicCookie)
		pos += 4

		return &net.UDPAddr{
			IP:   ip,
			Port: port,
		}, nil
	}

	return nil, fmt.Errorf("STUN reply did not include xor mapped address")
}
