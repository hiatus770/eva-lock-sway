#include "../include/pam_auth.h"
#include <security/pam_appl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

typedef struct {
    const char *password;
} pam_conv_data;

static int pam_conversation(int num_msg, const struct pam_message **msg,
                             struct pam_response **resp, void *appdata_ptr)
{
    pam_conv_data *d = appdata_ptr;
    *resp = calloc(num_msg, sizeof(struct pam_response));
    if (!*resp) return PAM_BUF_ERR;

    for (int i = 0; i < num_msg; i++) {
        if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF ||
            msg[i]->msg_style == PAM_PROMPT_ECHO_ON) {
            (*resp)[i].resp = strdup(d->password);
        }
    }
    return PAM_SUCCESS;
}

int eva_authenticate(const char *password)
{
    struct passwd *pw = getpwuid(getuid());
    const char *username = pw ? pw->pw_name : "";

    pam_conv_data data = { .password = password };
    struct pam_conv conv = { pam_conversation, &data };

    pam_handle_t *pamh = NULL;
    int ret = pam_start(EVA_PAM_SERVICE, username, &conv, &pamh);
    if (ret != PAM_SUCCESS) {
        fprintf(stderr, "pam_start failed: %s\n", pam_strerror(pamh, ret));
        return ret;
    }

    ret = pam_authenticate(pamh, 0);
    if (ret != PAM_SUCCESS) {
        fprintf(stderr, "pam_authenticate failed: %s\n", pam_strerror(pamh, ret));
    }

    pam_end(pamh, ret);
    return ret; // PAM_SUCCESS == 0
}
