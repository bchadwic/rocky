package main

import (
	"crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
	"log"
	"net"
	"time"
)

func main() {
	err := run()
	if err != nil {
		log.Fatalln(err.Error())
	}
}

func run() error {
	local := &net.UDPAddr{IP: net.IPv4zero, Port: 52838}

	conn, err := net.ListenUDP("udp", local)
	if err != nil {
		return err
	}
	defer conn.Close()

	public, err := getpublicaddress(conn)
	if err != nil {
		return err
	}

	fmt.Println(public)
	return nil

}

func getpublicaddress(conn *net.UDPConn) (*net.UDPAddr, error) {
	defer conn.SetReadDeadline(time.Time{})
	const host, port = "stun.l.google.com", 19302

	ips, err := net.LookupIP(host)
	if err != nil {
		return nil, err
	}

	if len(ips) == 0 {
		return nil, fmt.Errorf("could not resolve hostname: %s:%d", host, port)
	}

	req := make([]byte, 20)
	binary.BigEndian.PutUint16(req[0:], 0x0001)     // type
	binary.BigEndian.PutUint16(req[2:], 0x0000)     // len
	binary.BigEndian.PutUint32(req[4:], 0x2112A442) // magic cookie
	if _, err = rand.Read(req[8:]); err != nil {    // random transaction id
		return nil, fmt.Errorf("could not create random transaction id: %v", err)
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

		if _, err := conn.WriteTo(req, server); err != nil {
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

		public, err := parseSTUNResponse(resp[:n])
		if err != nil {
			errs = append(errs, err)
			continue
		}

		return public, nil
	}

	return nil, errors.Join(errs...)
}

func parseSTUNResponse(buf []byte) (*net.UDPAddr, error) {
	if len(buf) < 20 {
		return nil, fmt.Errorf("response too short")
	}

	const magicCookie = 0x2112A442
	if binary.BigEndian.Uint32(buf[4:8]) != magicCookie {
		return nil, fmt.Errorf("invalid magic cookie")
	}

	pos := 20
	for pos+4 <= len(buf) {
		attrType := binary.BigEndian.Uint16(buf[pos:])
		attrLen := binary.BigEndian.Uint16(buf[pos+2:])
		pos += 4
		if pos+int(attrLen) > len(buf) {
			break
		}

		if attrType == 0x0001 || attrType == 0x0020 { // MAPPED-ADDRESS or XOR-MAPPED-ADDRESS
			if attrLen < 8 {
				return nil, fmt.Errorf("attribute too short")
			}
			family := buf[pos+1]
			port := int(binary.BigEndian.Uint16(buf[pos+2 : pos+4]))
			ip := net.IPv4(buf[pos+4], buf[pos+5], buf[pos+6], buf[pos+7])
			if attrType == 0x0020 { // XOR-MAPPED-ADDRESS
				port ^= 0x2112
				for i := 0; i < 4; i++ {
					ip[i] ^= byte((magicCookie >> uint(24-8*i)) & 0xFF)
				}

			}
			if family != 0x01 { // IPv4 only
				return nil, fmt.Errorf("unsupported address family: %d", family)
			}
			return &net.UDPAddr{IP: ip, Port: port}, nil
		}
		pos += int(attrLen)
	}
	return nil, fmt.Errorf("no address found in response")
}
