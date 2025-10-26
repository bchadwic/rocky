package app

import (
	"container/heap"
	"net"
)

const (
	LocalOutboundAddress = iota
	LocalAddress
	ServerReflexiveAddress
)

type addrCandidate struct {
	priority int
	Addr     *net.UDPAddr
}

type addrCandidatesHeap []*addrCandidate

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

func (candidates *AddrCandidates) Push(candidate *addrCandidate) {
	if candidate.Addr == nil || candidate.Addr.IP == nil {
		return
	}

	ip := candidate.Addr.IP.String()
	if _, seen := candidates.m[ip]; seen {
		return
	}
	candidates.m[ip] = struct{}{}
	heap.Push(candidates.h, candidate)
}

func (candidates *AddrCandidates) Pop() *addrCandidate {
	candidate := heap.Pop(candidates.h).(*addrCandidate)
	ip := candidate.Addr.IP.String()
	delete(candidates.m, ip)
	return candidate
}

func (candidates *AddrCandidates) Len() int    { return candidates.h.Len() }
func (candidates *AddrCandidates) Empty() bool { return candidates.Len() == 0 }

func (h addrCandidatesHeap) Len() int               { return len(h) }
func (h addrCandidatesHeap) Less(i int, j int) bool { return h[i].priority < h[j].priority }
func (h addrCandidatesHeap) Swap(i int, j int)      { h[i], h[j] = h[j], h[i] }

func (a *addrCandidatesHeap) Push(v any) {
	*a = append(*a, v.(*addrCandidate))
}

func (h *addrCandidatesHeap) Pop() any {
	old := *h
	n := len(old)
	v := old[n-1]
	*h = old[:n-1]
	return v
}
