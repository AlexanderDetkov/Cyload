#pragma once
#include <stdio.h>
#include <string.h>

void stop(void) {
	while (1 == 1) {}
}

void replacechar(char *ix, char orig, char rep) {
	while ((ix = strchr(ix, orig)) != NULL) {
		*ix++ = rep;
	}
}