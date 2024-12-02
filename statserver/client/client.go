package main

import (
	"fmt"
	"net"
	"time"
)

// 192.168.157.1

func main() {
	fmt.Println("clienttt")
	for {
		conn, err := net.Dial("udp", "192.168.157.255:12345")
		if err != nil {
			panic(err)
		}
		defer conn.Close()

		_, err = conn.Write([]byte("penis"))
		if err != nil {
			panic(err)
		}
		time.Sleep(1 * time.Second)
	}
}
