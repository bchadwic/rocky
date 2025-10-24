package main

import (
	"net"
)

func getlocaladdresses() ([]net.UDPAddr, error) {
	ifaces, err := net.Interfaces()
	if err != nil {
		return nil, err
	}

	locals := []net.UDPAddr{}
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
			// fmt.Println(iface.Name, ip)
			locals = append(locals, net.UDPAddr{IP: ip, Port: 0})
		}
	}

	return locals, nil
}
