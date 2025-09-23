#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <limits.h>     // para PATH_MAX
#include <sys/types.h>  // para mkdir
#include <sys/stat.h>   // para mkdir
#include <libgen.h>     // para dirname

static volatile sig_atomic_t stop_requested = 0;

static void handle_sigterm(int signum) {
    (void)signum;
    stop_requested = 1;
}

static int mkdir_p(const char *path) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (len == 0)
        return -1;
    if (tmp[len - 1] == '/')
        tmp[len - 1] = '\0';
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0) {
                if (errno != EEXIST)
                    return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0) {
        if (errno != EEXIST)
            return -1;
    }
    return 0;
}

static FILE *open_logfile_try(const char *path) {
    char dir[PATH_MAX];
    // strncpy(dir, path, sizeof(dir));
    snprintf(dir, sizeof(dir), "%s", path);
    char *d = dirname(dir); // uses glibc's dirname which may modify buffer
    if (d == NULL) d = "/tmp";
    if (mkdir_p(d) != 0) {
        // ignore, we'll try to open anyway
    }
    FILE *f = fopen(path, "a");
    return f;
}

static char *choose_logfile(const char *preferred) {
    FILE *f = open_logfile_try(preferred);
    if (f) {
        fclose(f);
        return strdup(preferred);
    }
    const char *fallback = "/var/tmp/assignment_sensor.log";
    f = open_logfile_try(fallback);
    if (f) {
        fclose(f);
        return strdup(fallback);
    }
    return NULL;
}

static void hexdump(char *out, size_t outlen, const unsigned char *bin, size_t binlen) {
    size_t i;
    size_t pos = 0;
    for (i = 0; i < binlen && pos + 2 < outlen; ++i) {
        pos += snprintf(out + pos, outlen - pos, "%02x", bin[i]);
    }
    if (pos < outlen)
        out[pos] = '\0';
}

int main(int argc, char **argv) {
    int interval = 5;
    const char *logfile = NULL;
    const char *device = "/dev/urandom";

    int opt;
    while ((opt = getopt(argc, argv, "i:l:d:")) != -1) {
        switch (opt) {
        case 'i':
            interval = atoi(optarg);
            break;
        case 'l':
            logfile = optarg;
            break;
        case 'd':
            device = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-i interval] [-l logfile] [-d device]\n", argv[0]);
            return 2;
        }
    }

    if (interval <= 0) {
        fprintf(stderr, "invalid interval\n");
        return 2;
    }

    char *chosen = NULL;
    if (logfile && logfile[0] != '\0') {
        FILE *f = open_logfile_try(logfile);
        if (f) {
            fclose(f);
            chosen = strdup(logfile);
        } else {
            chosen = choose_logfile("/tmp/assignment_sensor.log");
            if (!chosen) {
                fprintf(stderr, "cannot open logfile %s and fallback failed\n", logfile);
                return 1;
            }
        }
    } else {
        chosen = choose_logfile("/tmp/assignment_sensor.log");
        if (!chosen) {
            fprintf(stderr, "cannot open preferred or fallback logfile\n");
            return 1;
        }
    }

    FILE *logf = fopen(chosen, "a");
    if (!logf) {
        fprintf(stderr, "failed to open logfile %s: %s\n", chosen, strerror(errno));
        free(chosen);
        return 1;
    }
    // make line buffered
    setvbuf(logf, NULL, _IOLBF, 0);

    // set up signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigterm;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Open device if not synthetic
    int devfd = -1;
    bool synthetic = false;
    if (strcmp(device, "synthetic") == 0) {
        synthetic = true;
    } else {
        devfd = open(device, O_RDONLY);
        if (devfd < 0) {
            fprintf(stderr, "failed to open device %s: %s\n", device, strerror(errno));
            fclose(logf);
            free(chosen);
            return 1;
        }
    }

    unsigned char buf[16];
    char hexbuf[33];
    while (!stop_requested) {
        ssize_t got = 0;
        if (synthetic) {
            unsigned int v = rand();
            // produce 4 bytes
            buf[0] = (v >> 24) & 0xff;
            buf[1] = (v >> 16) & 0xff;
            buf[2] = (v >> 8) & 0xff;
            buf[3] = (v >> 0) & 0xff;
            got = 4;
        } else {
            got = read(devfd, buf, sizeof(buf));
            if (got <= 0) {
                fprintf(logf, "%s ERROR reading %s: %s\n", "", device, strerror(errno));
                fflush(logf);
                if (errno == ENOENT) {
                    fclose(logf);
                    if (devfd >= 0) close(devfd);
                    free(chosen);
                    return 1;
                }
                sleep(interval);
                continue;
            }
        }

        // Timestamp in RFC3339 (ISO-8601)
        char timestr[64];
        time_t now = time(NULL);
        struct tm tm;
        gmtime_r(&now, &tm);
        strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%SZ", &tm);

        hexdump(hexbuf, sizeof(hexbuf), buf, (size_t)got);
        fprintf(logf, "%s %s\n", timestr, hexbuf);
        fflush(logf);

        for (int i = 0; i < interval && !stop_requested; ++i) {
            sleep(1);
        }
    }

    // cleanup
    fflush(logf);
    fclose(logf);
    if (devfd >= 0) close(devfd);
    free(chosen);
    return 0;
}
