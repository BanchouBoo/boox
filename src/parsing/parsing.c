#include <string.h>
#include <stdio.h>

#include "parsing.h"
#include "../xcb/selection.h"

char *format_output(char *src)
{
	static char buffer[4096];

	int offset = 0;
	while (*src) {
		char c = src[0];
		if (c == '%') {
			char nc = src[1];
			switch (nc) {
				case 'w':
					offset += sprintf(buffer+offset, "%d", selection.w);
					src++;
					break;
				case 'h':
					offset += sprintf(buffer+offset, "%d", selection.h);
					src++;
					break;
				case 'x':
					offset += sprintf(buffer+offset, "%d", selection.x);
					src++;
					break;
				case 'y':
					offset += sprintf(buffer+offset, "%d", selection.y);
					src++;
					break;
				default:
					strncpy(buffer+offset, src, 1);
					offset++;
			}
		} else if (c == '\\') {
			char nc = src[1];
			switch (nc) {
				case 'n':
					offset += sprintf(buffer+offset, "\n");
					src++;
					break;
				case 't':
					offset += sprintf(buffer+offset, "\t");
					src++;
					break;
				default:
					strncpy(buffer+offset, src, 1);
					offset++;
			}
		} else {
			strncpy(buffer+offset, src, 1);
			offset++;
		}
		src++;
	}
	return buffer;
}

void formatted_print(char *src)
{
	puts(format_output(src));
}
