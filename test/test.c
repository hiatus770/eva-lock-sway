#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "../include/files.h"
#include "../include/logger.h"

int main(){
    log_info("RUNNING TEST");

    char* teststring;

    teststring = get_path("makefile");
    log_debug("%s", teststring);
    teststring = get_shader_path("final.fs");
    log_debug("%s", teststring);
    teststring = get_texture_path("clock_4.png");
    log_debug("%s", teststring);
    teststring = get_font_path("matias.otf");
    log_debug("%s", teststring);

    return 0;
}
