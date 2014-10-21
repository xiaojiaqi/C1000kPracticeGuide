package main

import (
	"fmt"
	"net"
	"time"
)

var array []byte = make([]byte, 500)

func checkError(err error, info string) (res bool) {

	if err != nil {
		fmt.Println(info + "  " + err.Error())
		return false
	}
	return true
}

func Handler(conn net.Conn) {
//	fmt.Println("connection is connected from ...", conn.RemoteAddr().String())
	for {
		_, err := conn.Write(array)
		if err != nil {
			return
		}
		time.Sleep(10 * time.Second)
	}
}

func main() {

	for i := 0; i < 500; i += 1 {
		array[i] = 'a'
	}

	service := ":8888"
	tcpAddr, _ := net.ResolveTCPAddr("tcp4", service)
	l, _ := net.ListenTCP("tcp", tcpAddr)

	for {
//		fmt.Println("Listening ...")
		conn, _ := l.Accept()
		//fmt.Println("Accepting ...")
		go Handler(conn)
	}

}
