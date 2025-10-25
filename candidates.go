package main

import (
	"container/heap"
	"fmt"
	"net"
)

const (
	LocalOutboundAddress = iota
	LocalAddress
	ServerReflexiveAddress
)

type AddrCandidate struct {
	priority int
	addr     *net.UDPAddr
}

type addrCandidatesHeap []*AddrCandidate

type AddrCandidates struct {
	m map[string]struct{}
	h *addrCandidatesHeap
}

func NewAddrCandidates() *AddrCandidates {
	h := &addrCandidatesHeap{}
	heap.Init(h)

	return &AddrCandidates{
		m: map[string]struct{}{},
		h: h,
	}
}

func (candidates *AddrCandidates) Push(candidate *AddrCandidate) error {
	if candidate.addr == nil || candidate.addr.IP == nil {
		return fmt.Errorf("invalid ip address within candidate: %v", &candidate)
	}

	ip := candidate.addr.IP.String()
	if _, seen := candidates.m[ip]; seen {
		return fmt.Errorf("duplicate ip address being inserted: %v", &candidate)
	}
	candidates.m[ip] = struct{}{}
	heap.Push(candidates.h, candidate)
	return nil
}

func (candidates *AddrCandidates) Pop() *AddrCandidate {
	candidate := heap.Pop(candidates.h).(*AddrCandidate)
	ip := candidate.addr.IP.String()
	delete(candidates.m, ip)
	return candidate
}

func (candidates *AddrCandidates) Len() int    { return candidates.h.Len() }
func (candidates *AddrCandidates) Empty() bool { return candidates.Len() == 0 }

func (h addrCandidatesHeap) Len() int               { return len(h) }
func (h addrCandidatesHeap) Less(i int, j int) bool { return h[i].priority < h[j].priority }
func (h addrCandidatesHeap) Swap(i int, j int)      { h[i], h[j] = h[j], h[i] }

func (a *addrCandidatesHeap) Push(v any) {
	*a = append(*a, v.(*AddrCandidate))
}

func (h *addrCandidatesHeap) Pop() any {
	old := *h
	n := len(old)
	v := old[n-1]
	*h = old[:n-1]
	return v
}
