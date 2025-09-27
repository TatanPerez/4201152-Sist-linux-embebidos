# Reflection

I used the AI assistant to implement a small production-ready C program that
samples a mock sensor and logs timestamped values. I asked for a complete
project structure, including a systemd service, Makefile targeting `build/`,
tests, and AI evidence files.

What I accepted:
- Clear behavior for fallback when `/tmp` is not writable (attempt `/var/tmp`).
- Graceful SIGTERM handling (flush and close log, exit 0).
- Using `/dev/urandom` as default device and falling back to a counter when
  that device is unavailable.

What I rejected or adjusted:
- Kept the program single-threaded and simple rather than adding worker threads
  to keep it easy to audit and portable.

Validation steps I used or recommend:
- Built the program with `make` and inspected the produced binary at
  `build/assignment-sensor`.
- Manual tests described in `tests/README.md` to ensure logging, fallback,
  signal handling, and systemd restart behavior.

Overall this produced a compact, well-commented C program that fulfills the
assignment requirements while remaining simple to inspect and test.

---

## Additional AI assistance with ChatGPT

During development, I also consulted ChatGPT via the prompt at:

https://chatgpt.com/share/68d7286e-92bc-8001-97e6-b896a3bf43a4

which helped me clarify project structure and execution modes (manual vs systemd service), and guided me through common troubleshooting steps.

The iterative process involved refining commands, understanding service behavior, and integrating logging approaches.

This combined approach ensured a robust and well-documented solution.
