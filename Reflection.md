# Technical Reflection Report

**Module:** IE3090 - Network Programming  
**Student Name:** Hewavasan P H C  
**Student Registration Number:** IT24103185  
**Session ID Tag:** SID:5813  
**Assigned Listening Port:** 9410  
**Target Environment:** CentOS 10  
**Date:** October 2 2026  

---

## 1. TCP Sockets and Framing
Developing the RemoteOps client server model provideing hands on experience with POSIX socket functions (socket, bind, listen, accept). Because TCP is a connection oriented protocol without message boundaries, implementing clear framing techniques was important. Delimiters such as \n or \r\n were processed using buffer stripping and string parsing functions (strncmp, strtok) to reliably extract complete application commands from the byte stream.

## 2. Concurrency and Synchronization
To manage multiple client connections concurrently without blocking the primary listener thread, the agent spawns a dedicated POSIX worker thread (pthread_create) upon each accepted connection. Concurrent file writes to remoteops_IT24103185.log introduced potential data race conditions. This was resolved by protecting critical file I/O operations with a mutual exclusion lock (pthread_mutex_t), ensuring thread safety and preventing log corruption.

## 3. Security and Command Whitelisting
To prevent unappropreate command running vulnerabilities (such as unauthorized shell via laying EXEC bash), the system enforces strict input whitelisting. Only clear commands (DATE, WHOAMI, UPTIME, HOSTNAME) are permitted following authentication with OPS-3185. Unpermitted commands immediately trigger an error message (ERR 002 COMMAND NOT ALLOWED SID:5813), ensuring system isolation and safety.

## 4. Version Control and Build Automation
Version control was maintained through an incremental Git workflow (8 structured commits) tracking functional development points. Build management was automated using Makefile_185, which enforces strict GCC compilation standards (Wall ,Wextra,pthread) to produce reliable, warning free Linux binaries on CentOS 10.
