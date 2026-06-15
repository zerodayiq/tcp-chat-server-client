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
        
