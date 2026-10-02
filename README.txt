
SLIIT - Department of CSE
IE3090 - Computer Systems and Remote Management Assignment
RemoteOps Submission Package

Student Name            : Hiran
Student Registration No : IT24103185
Session ID Tag (SID)    : SID:5813
Personal Listening Port : 9410
Auth Token              : OPS-3185
Target OS               : CentOS Stream 10
Date                    : October 2, 2026



1. PACKAGE CONTENTS

This archive contains all required project artifacts:

1. agent_185.c                - Multithreaded C Agent server source code
2. controller_185.c           - C Client Controller CLI source code
3. Makefile_185               - Makefile build script
4. remoteops_IT24103185.log   - Thread-safe agent log file
5. Prompt_Log.pdf             - LLM interaction and prompt log history
6. Reflection.pdf             - Technical project reflection report
7. README.txt                 - Build instructions and package guide


2. ENVIRONMENT REQUIREMENTS

- Operating System: Linux (CentOS Stream 10 recommended)
- Compiler: GCC (GNU Compiler Collection) with POSIX thread support (-pthread)
- Build Tool: GNU Make
- Tools: netcat (nc), ss, git


3. COMPILATION INSTRUCTIONS

To compile both the Agent and Controller executables, run:

     make -f Makefile_185

To remove previous build binaries and clean the directory:

     make -f Makefile_185 clean


4. EXECUTION INSTRUCTIONS

[Step 1] Start the Agent Server:

Run the compiled agent binary (it will listen on port 9410):

     ./agent_185

(To run it in the background, use:  ./agent_185 &)

[Step 2] Launch the Controller Client:

In a new terminal window, connect using the controller client by providing the 
agent's IP address:

     ./controller_185 127.0.0.1

[Step 3] Alternate Testing via Netcat:

You can also connect directly using netcat:

     nc 127.0.0.1 9410


5. PROTOCOL COMMAND SUMMARY

- AUTH OPS-3185      : Authenticate active session (Required before running commands)
- SYSINFO            : Retrieve system load, memory, and uptime metrics
- LISTPROC           : Display active system processes
- EXEC DATE          : Run whitelisted command (DATE, WHOAMI, UPTIME, HOSTNAME)
- EXEC bash          : Test security whitelist (Returns ERR 002 COMMAND NOT ALLOWED)
- QUIT               : Disconnect session cleanly


