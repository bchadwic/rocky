package main

import (
	"net"
)

func getLocalAddresses() ([]AddrCandidate, error) {
	ifaces, err := net.Interfaces()
	if err != nil {
		return nil, err
	}

	locals := []AddrCandidate{}
	for _, iface := range ifaces {
		addrs, err := iface.Addrs()
		if err != nil {
			return nil, err
		}

		for _, addr := range addrs {
			ip, _, _ := net.ParseCIDR(addr.String())
			if ip.To4() == nil || ip.IsLoopback() || !ip.IsPrivate() {
				continue
			}

			locals = append(locals, AddrCandidate{priority: 3, addr: &net.UDPAddr{IP: ip, Port: 0}})
		}
	}

	return locals, nil
}
