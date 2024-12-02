package main

import (
	"fmt"
	"net"
)

func main() {
	pc, err := net.ListenPacket("udp4", ":12345")
	if err != nil {
		panic(err)
	}
	defer pc.Close()

	for {
		buf := make([]byte, 1024)
		n, addr, err := pc.ReadFrom(buf)
		if err != nil {
			panic(err)
		}
		fmt.Printf("%s sent this: %s\n", addr, buf[:n])
	}
}
