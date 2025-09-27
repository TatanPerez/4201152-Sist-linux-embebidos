Tests for assignment-sensor

1) Start service and check log output

- Build and install:
  sudo make install
  sudo cp systemd/assignment-sensor.service /etc/systemd/system/
  sudo systemctl daemon-reload
  sudo systemctl enable assignment-sensor
  sudo systemctl start assignment-sensor
- Check log file (default /tmp/assignment_sensor.log or /var/tmp fallback)
  tail -f /tmp/assignment_sensor.log

2) Force /tmp unwritable to test fallback

- Temporarily make /tmp unwritable (example using bind mount):
  sudo mount --bind /var/empty /tmp
- Start the program using /tmp as logfile path and ensure the file is created under /var/tmp

3) Stop service with systemctl stop and verify clean shutdown

- sudo systemctl stop assignment-sensor
- sudo systemctl status assignment-sensor
- Check last lines of log for graceful shutdown message

4) Start with invalid device to check error handling

- /usr/local/bin/assignment-sensor --device /dev/doesnotexist --logfile /tmp/x.log
- Expect non-zero exit and an error printed to stderr

5) Optional: kill process to test Restart=on-failure

- sudo systemctl stop assignment-sensor
- sudo systemctl start assignment-sensor
- Find pid and kill -9 <pid>
- Verify systemd restarts service (journalctl -u assignment-sensor)
