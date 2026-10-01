# Health Monitor

Fixed-capacity liveness policy. The watchdog is fed only when every mandatory source has checked in before its deadline. Recovery requests are injected; reboot rate limiting prevents uncontrolled restart loops.
