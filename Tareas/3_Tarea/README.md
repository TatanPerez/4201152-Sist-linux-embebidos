# Assignment: systemd sensor logger

This project implements a small systemd-friendly sensor logger written in C.

**What it does**
- Periodically samples a mock sensor (default `/dev/urandom`) and appends timestamp + hex value to a logfile.
- Prefers `/tmp/assignment_sensor.log` and falls back to `/var/tmp/assignment_sensor.log` if `/tmp` is not writable.
- Handles `SIGTERM` and shuts down cleanly.

**Repository layout**
- `src/` - Go source (`main.go`).
- `systemd/assignment-sensor.service` - systemd unit template.
- `tests/` - manual test instructions and scripts.
- `ai/` - AI evidence (prompt-log, reflection, provenance).
- `Makefile` - build/install/uninstall helpers.

Quick build & install (C / gcc)

```bash
make build
sudo make install
```

Install systemd unit and enable

```bash
sudo cp systemd/assignment-sensor.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now assignment-sensor.service
sudo journalctl -u assignment-sensor -f
```

Run binary by hand (for testing)

```bash
./build/assignment-sensor -i 2 -d /dev/urandom -l /tmp/assignment_sensor.log
```


CLI options
- `-i, --interval` (seconds): sampling interval. Default `5`.
- `-l, --logfile` (path): path to log file. If omitted the program prefers `/tmp/assignment_sensor.log` and falls back to `/var/tmp/assignment_sensor.log`.
- `-d, --device` (path or `synthetic`): device to read. Default `/dev/urandom`. `synthetic` emits pseudo-random integers and does not read a device file.

Design choices / sensor justification
- Using `/dev/urandom` (or `/dev/urandom`) as a mock sensor is convenient: always available on Linux, provides binary data to demonstrate logging. A `synthetic` option is included for environments where device files are restricted.

Permissions and `systemd` user
- The provided unit runs as root by default. To run as a non-root user set `User=` in the unit and ensure the user can write to the configured logfile location (e.g., choose a user-writable path or adjust permissions).

Uninstall

```bash
sudo systemctl disable --now assignment-sensor
sudo rm -f /etc/systemd/system/assignment-sensor.service
sudo systemctl daemon-reload
make uninstall
```

Tests
- See `tests/README.md` for suggested manual tests.
