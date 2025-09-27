/*
 * assignment-sensor.c
 *
 * Mock sensor logger.
 * Reads numeric samples from a device (default /dev/urandom) or a simple counter
 * and writes timestamped lines to a logfile. Handles SIGTERM for graceful
 * shutdown and falls back to /var/tmp if /tmp isn't writable.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <libgen.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t terminate = 0;

static void handle_sigterm(int signo) {
    (void)signo;
    terminate = 1;
}

static void print_usage(const char *prog) {
    fprintf(stderr,
            "%s --interval <seconds> --logfile <path> --device <path>\n",
            prog);
}

int main(int argc, char **argv) {
    const char *default_device = "/dev/urandom";
    const char *default_log = "/tmp/assignment_sensor.log";
    int interval = 5;
    const char *device_path = default_device;
    const char *log_path = default_log;

    static struct option longopts[] = {
        {"interval", required_argument, NULL, 'i'},
        {"logfile", required_argument, NULL, 'l'},
        {"device", required_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "i:l:d:h", longopts, NULL)) != -1) {
        switch (c) {
        case 'i':
            interval = atoi(optarg);
            if (interval <= 0) interval = 5;
            break;
        case 'l':
            log_path = optarg;
            break;
        case 'd':
            device_path = optarg;
            break;
        case 'h':
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Setup SIGTERM handler */
    struct sigaction sa = {0};
    sa.sa_handler = handle_sigterm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    /* Open device if possible */
    FILE *dev_fp = NULL;
    bool using_counter = false;

    dev_fp = fopen(device_path, "rb");
    if (!dev_fp) {
        /* If user explicitly requested a device and it failed, treat as fatal. */
        if (strcmp(device_path, default_device) != 0) {
            fprintf(stderr, "ERROR: unable to open device '%s': %s\n",
                    device_path, strerror(errno));
            return 2;
        }
        /* Default device not available: fallback to counter (not fatal) */
        fprintf(stderr,
                "WARNING: default device '%s' unavailable, falling back to counter.\n",
                default_device);
        using_counter = true;
    }

    /* Open logfile, with fallback from /tmp -> /var/tmp when necessary */
    char *chosen_log = strdup(log_path);
    if (!chosen_log) {
        fprintf(stderr, "ERROR: memory allocation failed\n");
        if (dev_fp) fclose(dev_fp);
        return 3;
    }

    FILE *log_fp = fopen(chosen_log, "a");
    if (!log_fp) {
        /* If path begins with /tmp, try /var/tmp fallback automatically */
        const char *tmp_prefix = "/tmp/";
        if (strncmp(chosen_log, tmp_prefix, strlen(tmp_prefix)) == 0) {
            size_t len = strlen(chosen_log) + 8;
            char *alt = malloc(len);
            if (alt) {
                snprintf(alt, len, "/var/tmp/%s", chosen_log + strlen(tmp_prefix));
                free(chosen_log);
                chosen_log = alt;
                log_fp = fopen(chosen_log, "a");
                if (log_fp) {
                    fprintf(stderr, "INFO: /tmp not writable, using fallback %s\n",
                            chosen_log);
                }
            }
        }
    }

    if (!log_fp) {
        fprintf(stderr, "ERROR: unable to open logfile '%s': %s\n",
                chosen_log, strerror(errno));
        free(chosen_log);
        if (dev_fp) fclose(dev_fp);
        return 4;
    }

    /* Ensure line-buffered logging and no partial lines. */
    setvbuf(log_fp, NULL, _IOLBF, 0);

    /* Main loop: sample and write lines until SIGTERM */
    uint64_t counter = 0;
    while (!terminate) {
        /* Timestamp in ISO-8601 (local time) */
        time_t now = time(NULL);
        struct tm tmnow;
        localtime_r(&now, &tmnow);
        char timestr[64];
        /* Example: 2025-09-26T15:04:05-0500 */
        if (strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S%z",
                     &tmnow) == 0) {
            strcpy(timestr, "1970-01-01T00:00:00+0000");
        }

        uint64_t value = 0;
        if (!using_counter) {
            uint64_t v = 0;
            size_t r = fread(&v, 1, sizeof(v), dev_fp);
            if (r == sizeof(v)) {
                value = v;
            } else {
                /* If device stops working mid-run, switch to counter and log */
                fprintf(stderr,
                        "WARNING: reading device failed, switching to counter\n");
                using_counter = true;
                value = counter++;
            }
        } else {
            value = counter++;
        }

        /* Write line: ISO-8601 timestamp | VALUE */
        fprintf(log_fp, "%s | %" PRIu64 "\n", timestr, value);
        fflush(log_fp);

        /* Sleep for interval seconds, but wake early on termination */
        for (int slept = 0; slept < interval && !terminate; ++slept) {
            sleep(1);
        }
    }

    /* Graceful shutdown: flush and close */
    fprintf(stderr, "INFO: received termination, shutting down gracefully...\n");
    fflush(log_fp);
    fclose(log_fp);
    free(chosen_log);
    if (dev_fp) fclose(dev_fp);

    return 0;
}
