sudo make install
sudo make uninstall
# Assignment Sensor Project

This repository implements `assignment-sensor`, a small C program that samples a
mock sensor and writes timestamped values to a logfile. It includes a systemd
service file, tests/instructions, and AI usage evidence.

## Prerequisites

- gcc (C compiler)
- make
- systemd (for service installation)

## Structure

- `src/assignment-sensor.c`: Main C source code
- `systemd/assignment-sensor.service`: systemd unit file
- `tests/`: Test scripts and manual test instructions
- `ai/`: AI usage evidence (prompt log, reflection, provenance)
- `Makefile`: Build, install, uninstall, clean

## Build

Build the binary (output at `build/assignment-sensor`):

```sh
make
```

## Install

Install binary into `/usr/local/bin` and copy the service file into systemd:

```sh
sudo make install
sudo cp systemd/assignment-sensor.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable assignment-sensor
sudo systemctl start assignment-sensor
```

Check status:

```sh
sudo systemctl status assignment-sensor
journalctl -u assignment-sensor -f
```

## Configuration

The program accepts flags at startup:

- `--interval <seconds>`: sampling interval in seconds (default 5)
- `--logfile <path>`: logfile path (default `/tmp/assignment_sensor.log`)
- `--device <path>`: device to read random data from (default `/dev/urandom`)

Examples:

```sh
/usr/local/bin/assignment-sensor --interval 2 --logfile /var/log/asensor.log
/usr/local/bin/assignment-sensor --device /dev/urandom --logfile /tmp/as.log
```

If `/tmp` is not writable the program will automatically fall back to a
corresponding path under `/var/tmp`.

## Tests

Manual test instructions are included in the `tests/` directory. Quick tests:

1) Normal logging

```sh
mkdir -p /tmp && /usr/local/bin/assignment-sensor --interval 1 --logfile /tmp/test_as.log &
sleep 3
tail -n +1 /tmp/test_as.log
kill %1
```

2) Fallback path when `/tmp` unwritable

```sh
sudo mount --bind /var/empty /tmp   # make /tmp unwritable for test (restore later)
/usr/local/bin/assignment-sensor --interval 1 --logfile /tmp/test_as.log &
sleep 2
ps aux | grep assignment-sensor
ls -l /var/tmp/test_as.log
sudo umount /tmp
```

3) SIGTERM graceful shutdown

Start the program in background, then `kill -TERM <pid>` and ensure the log is flushed and the process exits 0.

4) Failure path (invalid device)

```sh
/usr/local/bin/assignment-sensor --device /dev/doesnotexist --logfile /tmp/x.log
echo $?  # non-zero expected
```

5) Restart behavior

Use systemd `Restart=on-failure` to verify service restarts when killed with `SIGKILL`.

## Uninstall

```sh
sudo systemctl stop assignment-sensor
sudo systemctl disable assignment-sensor
sudo rm /etc/systemd/system/assignment-sensor.service
sudo systemctl daemon-reload
sudo make uninstall
```

## AI usage

See `ai/prompt-log.md`, `ai/reflection.md`, and `ai/provenance.json` for AI usage evidence.
