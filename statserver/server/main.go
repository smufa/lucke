package main

import (
	"encoding/json"
	"fmt"
	"lucke/m/circularbuffer"
	"net"
	"net/http"
	"time"
)

const HISTORY_LEN = 20

type StatPacket struct {
	Universe         int64   `json:"universe"`
	HeapSize         int64   `json:"heap_size"`
	HeapFree         int64   `json:"heap_free"`
	LocalIP          string  `json:"local_ip"`
	SSID             string  `json:"ssid"`
	Rssi             int64   `json:"rssi"`
	LastDMXFramerate int64   `json:"last_DMX_framerate"`
	First5_Leds      []int64 `json:"first_5_leds"`
}

type StatStore struct {
	Universe      int64                          `json:"universe"`
	HeapSize      int64                          `json:"heap_size"`
	HeapFree      int64                          `json:"heap_free"`
	LocalIP       string                         `json:"local_ip"`
	SSID          string                         `json:"ssid"`
	Rssi          *circularbuffer.CircularBuffer `json:"rssi"`
	DMXFramerate  *circularbuffer.CircularBuffer `json:"DMXFramerate"`
	First5_Leds   []int64                        `json:"first_5_leds"`
	LastHeartbeat time.Time                      `json:"lastHeartbeat"`
}

var stats = make(map[int64]StatStore)

func packetIngest() {

	pc, err := net.ListenPacket("udp4", ":12345")
	if err != nil {
		panic(err)
	}
	defer pc.Close()

	for {
		buf := make([]byte, 1024)
		n, _, err := pc.ReadFrom(buf)
		if err != nil {
			fmt.Println("ERR reading", err)
			continue
		}
		// str := fmt.Sprintf("%s\n", buf[:n])
		// fmt.Println(str)

		res := StatPacket{}
		err2 := json.Unmarshal(buf[:n], &res)

		if err2 != nil {
			fmt.Println("err parsing", err2)
			continue
		}

		val, ok := stats[res.Universe]
		// If the key exists
		if ok {

			stats[res.Universe] = StatStore{
				Universe:      res.Universe,
				HeapSize:      res.HeapSize,
				HeapFree:      res.HeapFree,
				LocalIP:       res.LocalIP,
				SSID:          res.SSID,
				Rssi:          val.Rssi.Add(res.Rssi),
				DMXFramerate:  val.DMXFramerate.Add(res.LastDMXFramerate),
				First5_Leds:   res.First5_Leds,
				LastHeartbeat: time.Now(),
			}

		} else {
			stats[res.Universe] = StatStore{
				Universe:      res.Universe,
				HeapSize:      res.HeapSize,
				HeapFree:      res.HeapFree,
				LocalIP:       res.LocalIP,
				SSID:          res.SSID,
				Rssi:          circularbuffer.NewCircularBufferWithVals(HISTORY_LEN, res.Rssi),
				DMXFramerate:  circularbuffer.NewCircularBufferWithVals(HISTORY_LEN, res.LastDMXFramerate),
				First5_Leds:   res.First5_Leds,
				LastHeartbeat: time.Now(),
			}
		}

		// fmt.Println("%s\n", stats)
		// fmt.Println(time.Now())
		fmt.Println(".")
	}
}

func apiServer() {
	// Define the handler function for the root endpoint
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		// Set the response header to indicate JSON content
		w.Header().Set("Content-Type", "application/json")
		// Set CORS headers
		w.Header().Set("Access-Control-Allow-Origin", "*")                   // Allow requests from all origins
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, OPTIONS") // Allowed methods
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type")       // Allowed headers

		// Handle OPTIONS preflight requests
		if r.Method == http.MethodOptions {
			w.WriteHeader(http.StatusNoContent) // Respond with 204 No Content
			return
		}

		// Encode the response as JSON and write it to the response writer
		json.NewEncoder(w).Encode(stats)
	})

	// Start the HTTP server on port 8080
	fmt.Println("listening on localhost:8080")
	http.ListenAndServe(":8080", nil)
}

// func websiteServer() {
// 	// // Define the handler function for the root endpoint
// 	// fs := http.FileServer(http.Dir("./ledMon/dist"))
// 	// http.Handle("/", fs)

// 	fmt.Println("webserver on 8080")
// 	http.ListenAndServe(":8080", http.FileServer(http.Dir("./ledMod/dist")))
// }

func main() {

	// start the loop for ingesting logs
	go packetIngest()
	go apiServer()
	// go webswiteServer()

	// infinite loop
	for {
		time.Sleep(100 * time.Second)
	}
}
