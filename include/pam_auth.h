#pragma once

// PAM service name — Arch/CachyOS: "system-local-login"; other distros may use "login"
#define EVA_PAM_SERVICE "system-local-login"

// Returns 0 (PAM_SUCCESS) on successful authentication, nonzero on failure.
int eva_authenticate(const char *password);
