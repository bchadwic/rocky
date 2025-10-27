package app

import (
	"crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"time"
)

func GetServerReflexiveAddress() (*addrCandidate, *addrCandidate, error) {
	const (
		Host = "stun.l.google.com"
		Port = 19302
	)

	ips, err := net.LookupIP(Host)
	if err != nil {
		return nil, nil, err
	}

	if len(ips) == 0 {
		return nil, nil, fmt.Errorf("could not resolve hostname: %s:%d", Host, Port)
	}

	try := func(conn *net.UDPConn) (*addrCandidate, *addrCandidate, error) {
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

		return &addrCandidate{
				priority: ServerReflexiveAddress,
				Addr:     publicAddr,
			},
			&addrCandidate{
				priority: LocalOutboundAddress,
				Addr:     &net.UDPAddr{IP: localIp, Port: 0}, // all local address ports should be hardcoded to zero
			}, nil
	}

	errs := []error{}
	for _, ip := range ips {
		if ip.To4() == nil {
			continue
		}

		attempt := &net.UDPAddr{IP: ip, Port: Port}
		conn, err := net.DialUDP("udp4", &net.UDPAddr{IP: net.IPv4zero, Port: Port}, attempt)
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
		Binding      uint16 = 0x0001
		NoAttributes uint16 = 0x0000
		MagicCookie  uint32 = 0x2112A442
	)

	buf := make([]byte, 20)
	binary.BigEndian.PutUint16(buf[0:], Binding)
	binary.BigEndian.PutUint16(buf[2:], NoAttributes)
	binary.BigEndian.PutUint32(buf[4:], MagicCookie)
	if _, err := rand.Read(buf[8:]); err != nil { // random transaction id
		return nil, fmt.Errorf("could not create random transaction id for STUN message: %v", err)
	}
	return buf, nil
}

func unpack(buf []byte) (*net.UDPAddr, error) {
	const (
		HeaderLength            = 20
		Binding          uint16 = 0x0101
		XorMappedAddress uint16 = 0x0020
		MagicCookie      uint32 = 0x2112A442
		IPv4             uint8  = 0x01
	)
	ntohs := binary.BigEndian.Uint16
	ntohl := binary.BigEndian.Uint32

	if len(buf) < HeaderLength {
		return nil, fmt.Errorf("STUN reply less than 20 bytes: % X", buf)
	}

	pos := 0
	if typ := ntohs(buf[pos:]); typ != Binding {
		return nil, fmt.Errorf("STUN reply type is not binding reply: % X", buf)
	}
	pos += 2

	if length := ntohs(buf[pos:]); int(length) > len(buf)-HeaderLength {
		return nil, fmt.Errorf("STUN reply attributes exceed buffer size: % X", buf)
	}
	pos += 2

	if cookie := ntohl(buf[pos:]); cookie != MagicCookie {
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
		if typ != XorMappedAddress {
			pos += int(length)
			continue
		}

		if pos+8 > len(buf) {
			return nil, fmt.Errorf("STUN xor mapped address attribute value is improperly sized: % X", buf[pos:])
		}

		// skip first byte
		if buf[pos+1] != IPv4 {
			return nil, fmt.Errorf("STUN reply address family is unsupported: %02X", buf[pos])
		}
		pos += 2

		port := int(ntohs(buf[pos:]) ^ uint16(MagicCookie>>16))
		pos += 2

		ip := make(net.IP, 4)
		binary.BigEndian.PutUint32(ip, ntohl(buf[pos:])^MagicCookie)
		pos += 4

		return &net.UDPAddr{
			IP:   ip,
			Port: port,
		}, nil
	}

	return nil, fmt.Errorf("STUN reply did not include xor mapped address")
}
