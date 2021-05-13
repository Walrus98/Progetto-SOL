#ifndef SERVER_CACHE_HANDLER_H
#define SERVER_CACHE_HANDLER_H

void inizialize_policy(int replacementPolicy);

char *replacement_file_cache();
void insert_file_cache(char *file);
void insert_update_file_cache(char *file);
int contains_file_cache(char *file);
void destroy_cache();
void print_cache();

#endif