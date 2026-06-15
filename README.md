# tcp-chat-server-client
A simple multithreaded TCP chat system written in C++. It consists of a server and multiple clients communicating in real-time over sockets.  ⚠️ This project is for educational purposes only and demonstrates: - TCP socket programming - Multithreading - Basic client-server architecture

# Architecture

        ┌───────────────┐
        │   Client #1   │
        └──────┬────────┘
               │
        ┌──────▼────────┐
        │   Client #2   │
        └──────┬────────┘
               │
        ┌──────▼────────┐
        │   Client #N   │
        └──────┬────────┘
               │
        ┌──────▼────────┐
        │ TCP SERVER    │
        │ - handlesrecv │
        │ - broadcast   │
        │ - threads     │
        └───────────────┘
        
Client → Server → Broadcast → All Clients        


## Features

- 🔌 TCP socket-based communication
- 👥 Multi-client support
- 🧵 Thread per client handling
- 📡 Broadcast messaging system
- 🛑 Graceful client disconnect handling
- 💬 Username support (client-side)

## Project Structure

.
├── chat.cpp        # Combined server + client implementation
└── README.md

## Build

g++ main.cpp -o chat -pthread

## Run Server

./chat server

## Run Client

./chat client <SERVER_IP>


## Limitations

- ❌ No encryption (plaintext traffic)
- ❌ No authentication system
- ❌ Not scalable (thread-per-client model)
- ❌ No message framing protocol (TCP stream limitation)
- ❌ No logging system
- ❌ No rate limiting or flood protection

---

MIT License

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
