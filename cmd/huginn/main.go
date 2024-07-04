package main
import (
	"fmt"
	"os"
	"huginn/internal/wakeonlan"
)

func main() {
	// get mac from args
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <MAC address>\n", os.Args[0])
		os.Exit(1)
	}

	mac := os.Args[1]
	
	if err := wakeonlan.SendWOL(mac); err != nil { 
		fmt.Fprintf(os.Stderr, "Failed to send Wake-on-LAN packet: %v\n", err)
		os.Exit(1)
	}

	fmt.Printf("Wake-on-LAN packet sent to %s\n", mac)
}
