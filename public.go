package main

import (
	"crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"time"
)

func getpublicaddress(conn *net.UDPConn) (*net.UDPAddr, error) {
	defer conn.SetReadDeadline(time.Time{})
	const (
		host = "stun.l.google.com"
		port = 19302
	)

	ips, err := net.LookupIP(host)
	if err != nil {
		return nil, err
	}

	if len(ips) == 0 {
		return nil, fmt.Errorf("could not resolve hostname: %s:%d", host, port)
	}

	errs := []error{}
	for _, ip := range ips {
		if ip.To4() == nil {
			continue
		}

		server := &net.UDPAddr{
			IP:   ip,
			Port: port,
		}

		buf, err := pack()
		if err != nil {
			return nil, err
		}

		if _, err := conn.WriteTo(buf, server); err != nil {
			errs = append(errs, err)
			continue
		}

		resp := make([]byte, 512)
		conn.SetReadDeadline(time.Now().Add(800 * time.Millisecond))
		n, err := conn.Read(resp)
		if err != nil {
			errs = append(errs, err)
			continue
		}

		public, err := unpack(resp[:n])
		if err != nil {
			errs = append(errs, err)
			continue
		}

		return public, nil
	}

	return nil, errors.Join(errs...)
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
