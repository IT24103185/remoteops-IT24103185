# Technical Reflection Report

**Module:** IE3090 — Computer Systems & Remote Management  
**Student Name:** Hiran  
**Student Registration Number:** IT24103185  
**Session ID Tag:** SID:5813  
**Assigned Listening Port:** 9410  
**Target Environment:** CentOS Stream 10  
**Date:** October 2, 2026  

---

## 1. TCP Sockets & Framing
Developing the RemoteOps client-server model provided hands-on experience with POSIX socket APIs (`socket`, `bind`, `listen`, `accept`). Because TCP is a stream-oriented protocol without inherent message boundaries, implementing explicit framing techniques was essential. Delimiters such as `\n` or `\r\n` were processed using buffer stripping and string parsing functions (`strncmp`, `strtok`) to reliably extract complete application commands from the byte stream.

## 2. Concurrency & Synchronization
To manage multiple client connections concurrently without blocking the primary listener thread, the agent spawns a dedicated POSIX worker thread (`pthread_create`) upon each accepted connection. Concurrent file writes to `remoteops_IT24103185.log` introduced potential data race conditions. This was resolved by protecting critical file I/O operations with a mutual exclusion lock (`pthread_mutex_t`), ensuring thread safety and preventing log corruption.

## 3. Security & Command Whitelisting
To prevent arbitrary command execution vulnerabilities (such as unauthorized shell spawning via `EXEC bash`), the system enforces strict input whitelisting. Only explicit commands (`DATE`, `WHOAMI`, `UPTIME`, `HOSTNAME`) are permitted following authentication with `OPS-3185`. Unpermitted commands immediately trigger an error response (`ERR 002 COMMAND NOT ALLOWED SID:5813`), ensuring system isolation and execution safety.

## 4. Version Control & Build Automation
Version control was maintained through an incremental Git workflow (8+ structured commits) tracking functional development milestones. Build management was automated using `Makefile_185`, which enforces strict GCC compilation standards (`-Wall -Wextra -pthread`) to produce reliable, warning-free Linux binaries on CentOS Stream 10.
