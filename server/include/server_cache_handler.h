#ifndef SERVER_CACHE_HANDLER_H
#define SERVER_CACHE_HANDLER_H

#include "server_storage.h"
#include "icl_hash.h"

void inizialize_policy(int replacementPolicy);

void replacement_file_cache(icl_hash_t *storage);
void inset_file_cache(File *file);
void insert_update_file_cache(File *file);
int get_file_cache(File *file);
void destroy_cache();
void print_cache();

#endif