#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_lex.h"

int main() {
    // Read file example.json
	FILE *fp = fopen("example.json", "r");
	if (fp == NULL) {
		printf("Error opening file!\n");
		return 1;
	}

	// Read file into buffer
	char *buffer = NULL;
	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buffer = malloc(length);
	if (buffer == NULL) {
		printf("Error allocating memory!\n");
		return 1;
	}
	fread(buffer, 1, length, fp);
	fclose(fp);

	// Print buffer
	printf("%s\n", buffer);

	// Lex
	uint32_t tokens_len = 1000;
	json_token_t *tokens = malloc(tokens_len * sizeof(json_token_t));
	uint32_t num_tokens = 0;
	json_lex_init();
	if (json_lex(buffer, length, tokens, &num_tokens, tokens_len) != JSON_RETVAL_OK) {
		printf("Error lexing!\n");
		return 1;
	}
	for (uint32_t i = 0; i < num_tokens; i++) {
		const int str_len = 255;
		char str[str_len];
		json_get_token_str_repr(&tokens[i], str, str_len);
		printf("[%s]\n", str);
	}

	// Free tokens
	free(tokens);

	// Free buffer
	free(buffer);

    return 0;
}
