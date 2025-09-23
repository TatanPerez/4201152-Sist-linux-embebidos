Manual tests for assignment-sensor (C implementation)

1) Build and install

```bash
make build
sudo make install
```

2) Install systemd unit and enable

```bash
sudo cp systemd/assignment-sensor.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now assignment-sensor.service
sudo journalctl -u assignment-sensor -f
```

3) Verify logs

Check `/tmp/assignment_sensor.log` for lines of the form:

```
2025-09-19T12:34:56Z d4f0c2a1...
```

4) Simulate `/tmp` not writable -> verify fallback to `/var/tmp`

Make `/tmp` non-writable (do carefully and restore afterwards):

```bash
sudo chmod a-w /tmp
sudo systemctl restart assignment-sensor
# look into /var/tmp/assignment_sensor.log for entries
sudo chmod a+w /tmp
```

5) Stop service gracefully

```bash
sudo systemctl stop assignment-sensor
# Service should exit and flush logs
```

6) Start with non-existent device

Edit the unit to use a non-existent device and start; the service should fail with non-zero exit and log to journal.
