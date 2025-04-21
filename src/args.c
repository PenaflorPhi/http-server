#include "server.h"
#include <stdio.h>
#include <string.h>

static void initailize_setting(Settings *settings) { settings->directory = NULL; }

Settings parse_arguments(int argc, char **argv) {
    Settings settings;
    initailize_setting(&settings);

    for (int i = 1; i < argc; ++i) {
        if (strcmp("--directory", argv[i]) == 0) {
            settings.directory = argv[i + i];
            continue;
        }
    }

    return settings;
}
