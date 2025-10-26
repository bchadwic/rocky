package app

import (
	"net"
)

func GetLocalAddresses() ([]addrCandidate, error) {
	ifaces, err := net.Interfaces()
	if err != nil {
		return nil, err
	}

	locals := []addrCandidate{}
	for _, iface := range ifaces {
		addrs, err := iface.Addrs()
		if err != nil {
			return nil, err
		}

		if iface.Flags&net.FlagUp == 0 || iface.Flags&net.FlagLoopback != 0 {
			continue
		}

		for _, addr := range addrs {
			ip, _, err := net.ParseCIDR(addr.String())
			if err != nil || ip.To4() == nil || !ip.IsGlobalUnicast() {
				continue
			}

			locals = append(locals, addrCandidate{priority: LocalAddress, Addr: &net.UDPAddr{IP: ip, Port: 0}})
		}
	}

	return locals, nil
}
