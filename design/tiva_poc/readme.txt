Copyright 2025, Ahmed Abdulla. All Rights Reserved.

Current Status:
FINISHED! Only optional cleanup/refactoring is left to do.

Future improvements:
Move most svc call implementations to internal.c
Move svc call wrappers to svc.c
Move svcCallIsr to svc.c
Move most of shell implementation to userspace by doing:
    1. svc return data struct with only relevant info
    2. shell userspace implement all functionality using data struct
    3. Convert putsUart0 to userspace function
