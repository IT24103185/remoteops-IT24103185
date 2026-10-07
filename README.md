# RemoteOps Remote Management System

**Student Name:** Hewavasan P H C  
**Registration Number:** IT24103185  
**Session ID Tag:** SID:5813  
**Listening Port:** 9410  
**Auth Token:** OPS 3185  
**Target OS:** CentOS Stream 10  

---

## Deliverables Included
* `agent_185.c` - Multi-threaded C socket server
* `controller_185.c` - C CLI controller client
* `Makefile_185` - Project build script
* `remoteops_IT24103185.log` - Activity log file
* `README.txt` - Text package summary
* `Prompt_Log.pdf` - AI interaction log
* `Reflection.pdf` - Technical reflection report

---

## Quick Start
```bash
# Build binaries
make -f Makefile_185

# Start Agent
./agent_185

# Start Controller
./controller_185 127.0.0.1
