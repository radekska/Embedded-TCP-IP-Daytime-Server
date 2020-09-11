# Embedded TCP/IP Daytime Server
This is a repository for embedded C application build on top of **STM32 Nucleo-F401RE** with **Wiznet W5100** chip on Arduino Shield. The daytime server is running on port 13 and is scheduled by FreeRTOS to handle multiple clients at the same time.
For more detailed description see [documentation](https://github.com/radekska/AMK_TCP-IP_SERV/blob/master/TCP_IP_Daytime_Server-Dokumentacja.pdf) (in Polish).

## Description
The server opens the *listening socket* in the main thread. When the remote client wishes to connect to the server, it retreives the child socket from the queue of available worker sockets, and it is creating a new FreeRTOS task, which is handling the connection with the peer. The child socket is sending the daytime with one second resolution.

## Built With

* [C](http://www.open-std.org/jtc1/sc22/wg14/) –  implementation language
* [FreeRTOS](https://www.freertos.org/) – embedded operating system
* [Wiznet ioLiblary](https://github.com/Wiznet/ioLibrary_Driver) – driver and socket API for Wiznet chip
* [STM32 Nucleo-F401RE](https://www.st.com/en/evaluation-tools/nucleo-f401re.html) – used developement board
* [Wiznet W5100](https://www.wiznet.io/product-item/w5100/) – Ethernet interface chip

## Authors

* **Kamil Kaliś** – [kamkali](https://github.com/kamkali)
* **Radosław Skałbania** – [radekska](https://github.com/radekska)
* **Szymon Stolarski** - [szymonsto](https://github.com/szymonsto)

## License

This project is using Wiznet liblary, which is licensed under the Wiznet license - see the [license](https://github.com/radekska/AMK_TCP-IP_SERV/blob/master/server/ioLibrary_Driver/license.txt) file for details

## Board Connection
![pic1](https://user-images.githubusercontent.com/50112357/92881112-f8da5880-f40e-11ea-8ccd-772875d4f6e1.png)
