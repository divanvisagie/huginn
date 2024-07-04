package wakeonlan
import (
	"errors"
	"net"
)

const (
	broadcastIP   = "255.255.255.255"
	wolPort       = 9
	macAddressLen = 6
)

func parseMAC(mac string) ([]byte, error) {
	hw, err := net.ParseMAC(mac)
	if err != nil {
		return nil, err
	}
	if len(hw) != macAddressLen {
		return nil, errors.New("invalid MAC address length")
	}
	return hw, nil
}

func createMagicPacket(mac []byte) []byte {
	packet := make([]byte, 6+16*macAddressLen)
	for i := 0; i < 6; i++ {
		packet[i] = 0xFF
	}
	for i := 0; i < 16; i++ {
		copy(packet[6+i*macAddressLen:], mac)
	}
	return packet
}

func SendWOL(mac string) error {
	hw, err := parseMAC(mac)
	if err != nil {
		return err
	}

	packet := createMagicPacket(hw)

	conn, err := net.DialUDP("udp4", nil, &net.UDPAddr{
		IP:   net.ParseIP(broadcastIP),
		Port: wolPort,
	})
	if err != nil {
		return err
	}
	defer conn.Close()

	_, err = conn.Write(packet)
	if err != nil {
		return err
	}

	return nil
}
