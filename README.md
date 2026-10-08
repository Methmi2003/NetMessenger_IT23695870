# NetMessenger – IT23695870

## Student Details

Name: Methmi Dilshara
Registration Number: IT23695870
Module: IE3010 Network Programming

## Personalisation Details

Numeric Registration Number: 23695870
Last Four Digits: 5870
TCP Port: 11870
Node ID: NID:6958

Server Source File: server_5870.c
Client Source File: client_5870.c
Makefile: Makefile_5870
Log File: netmsg_IT23695870.log
Server Storage Path: ./storage/IT23695870/<sender_username>/<filename>

## Project Description

NetMessenger is a multi-client TCP/IP chat and file-sharing application
implemented in C. The system uses a central TCP server and multiple
clients communicating through a newline-oriented application protocol.

The server supports client registration, user listing, broadcast messaging,
private messaging, chat rooms, room messaging, file transfer, graceful
disconnect handling, and unexpected disconnect cleanup. Multiple clients
are handled concurrently using POSIX threads, while shared client and room
data is protected using mutexes.
