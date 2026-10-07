# AI Prompt & Interaction Log

**Module:** IE3090 — Computer Systems & Remote Management  
**Student Name:** Hiran  
**Student Registration Number:** IT24103185  
**Session ID Tag:** SID:5813  
**Assigned Listening Port:** 9410  
**Authentication Token:** OPS-3185  
**Target Environment:** CentOS Stream 10  
**Date:** October 2, 2026  

---

### Academic Integrity & Verification Statement
All AI-generated recommendations, code structures, and command scripts documented in this log were independently compiled, tested, and validated in a CentOS Stream 10 virtual machine environment. Source code logic was modified and verified to adhere to the assignment parameters, including socket port binding (`9410`), session tag validation (`SID:5813`), and authentication handling (`OPS-3185`).

---

### Phase 1: TCP Socket Server & Multi-threaded Architecture

* **User Prompt:**
  > "How do I set up a multi-threaded TCP socket server in C running on CentOS 10 that binds to custom port 9410 and handles multiple client connections concurrently?"

* **AI Guidance Received:**
  Provided POSIX socket setup workflow using `socket()`, `bind()`, `listen()`, and `accept()`. Recommended spawning worker threads using `pthread_create()` to manage concurrent client connections without blocking the main listener.

* **Verification & Testing:**
  Implemented in `agent_185.c`. Tested local listening status in CentOS terminal using `ss -tlnp | grep 9410` and verified successful binding on port `9410`.

---

### Phase 2: Custom Protocol Parsing & Authentication

* **User Prompt:**
  > "How do I implement custom protocol command parsing for SYSINFO, LISTPROC, EXEC, and AUTH token OPS-3185 with SID:5813 tags appended to responses?"

* **AI Guidance Received:**
  Suggested string tokenization and prefix matching via `strncmp()` to parse client inputs. Advised appending `SID:5813` to formatted server string responses using `snprintf()`. Recommended setting an authentication flag per connection thread to enforce pre-auth access checks.

* **Verification & Testing:**
  Verified with `nc 127.0.0.1 9410`. Confirmed that executing commands before sending `AUTH OPS-3185` returns `ERR 001 AUTH FAILED SID:5813`.

---

### Phase 3: Command Whitelisting & Mutex Synchronization

* **User Prompt:**
  > "How do I make logging to remoteops_IT24103185.log thread-safe with pthread_mutex_t and restrict command execution to a safe whitelist?"

* **AI Guidance Received:**
  Provided synchronization logic wrapping file I/O operations inside `pthread_mutex_lock(&log_mutex)` and `pthread_mutex_unlock(&log_mutex)`. Suggested strict equality checks for `EXEC` commands allowing only `DATE`, `WHOAMI`, `UPTIME`, and `HOSTNAME`.

* **Verification & Testing:**
  Executed `EXEC bash` via client to verify security enforcement; server correctly blocked execution with `ERR 002 COMMAND NOT ALLOWED SID:5813`. Inspected `remoteops_IT24103185.log` with `cat` to confirm sequential, uncorrupted log entries across multiple threads.

---

### Phase 4: Build Automation & Deployment

* **User Prompt:**
  > "Write a Makefile that compiles agent_185.c and controller_185.c using GCC with flags -Wall -Wextra -pthread."

* **AI Guidance Received:**
  Generated Makefile rules specifying target executables (`agent_185`, `controller_185`), compiler flags (`-Wall -Wextra -pthread`), and a `clean` target.

* **Verification & Testing:**
  Executed `make -f Makefile_185 clean && make -f Makefile_185` on CentOS 10; confirmed clean compilation with zero warnings or errors.
