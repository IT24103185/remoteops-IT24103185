**Module:** IE3090 — Computer Systems & Remote Management  
**Student Name:** Hiran  
**Student Registration Number:** IT24103185  
**Email:** it24103185@my.sliit.lk  
**Session ID Tag:** SID:5813  
**Assigned Listening Port:** 9410  
**Authentication Token:** OPS-3185  
**Target Platform:** CentOS Stream 10  
**Date:** October 7, 2026  

---

## 1. Executive Summary & Design Goals
The RemoteOps system is a lightweight, concurrent client-server tool built in C for remote Linux administration on CentOS Stream 10. The core architecture focuses on thread safety, strict security controls, custom protocol framing, and low-level POSIX socket handling.

### Primary Objectives
* **Reliable TCP Communication:** Bind server socket (`agent_185.c`) to dedicated port `9410`.
* **Session & Identity Tracking:** Tag all client-server communications with `SID:5813`.
* **Access Control & Security:** Mandatory authentication using `OPS-3185` token and strict command whitelisting.
* **Concurrency & Synchronization:** Multi-client handling using `pthread` and mutex-locked logging to `remoteops_IT24103185.log`.

---

## 2. System Architecture & Component Mapping

```text
+-----------------------+              +-----------------------------------+
|  Controller (Client)  |              |          Agent (Server)           |
|   controller_185.c    |              |            agent_185.c            |
+-----------------------+              +-----------------------------------+
            |                                           |
            |---- (1) TCP Connect (Port 9410) --------->|
            |---- (2) AUTH OPS-3185 ------------------->| ---> Check Token
            |<--- (3) OK AUTH SUCCESS SID:5813 ---------|
            |                                           |
            |---- (4) EXEC DATE ----------------------->| ---> Whitelist Check
            |                                           | ---> Lock Mutex & Log
            |<--- (5) [Output] SID:5813 ----------------|      remoteops_IT24103185.log
