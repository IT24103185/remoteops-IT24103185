cat << 'EOF' > Design_Diary.md
# Design Diary - RemoteOps System Architecture

**Module:** IE3090 - Network Programming  
**Student Name:** Hewavasan P H C  
**Student Registration Number:** IT24103185  
**Session ID Tag:** SID:5813  
**Assigned Listening Port:** 9410  
**Authentication Token:** OPS-3185  
**Target Platform:** CentOS 10  
**Date:** October 7 2026  

---

## 1. Executive Summary and Design Goals
The RemoteOps system is a lightweight, concurrent client server tool built in C for remote Linux administration on CentOS Stream 10. The core architecture focuses on thread safety, strict security controls, custom protocol framing, and low level POSIX socket handling.

### Primary Objectives
- **Reliable TCP Communication** - Bind server socket (`agent_185.c`) to dedicated port 9410.
- **Session and Identity Tracking** - Tag all client server communications with `SID:5813`.
- **Access Control and Security** - Mandatory authentication using `OPS-3185` token and strict command whitelisting.
- **Concurrency and Synchronization** - Multi client handling using `pthread` and mutex locked logging to `remoteops_IT24103185.log`.

---

## 2. System Architecture and Component Mapping

Controller (client)                             Agent (Server)
|                                             |
|---------- TCP Connect (Port 9410) --------->|
|                                             |
|-------------- AUTH OPS-3185 --------------->| Check Token
|<---------- OK AUTH SUCCESS SID:5813 --------|
|                                             |
|---------------- EXEC DATE ----------------->| Whitelist Check
|                                             | lock mutex and log
|<------------ Output [SID = 5813] -----------|
|                                             |

