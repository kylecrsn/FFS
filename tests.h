#ifndef TESTS_H
#define TESTS_H

#include "global.h"

void make_test_data();
void clear_file(const char *fn);
void dump_disk_to_file(const char *fn, const char *fl);
void comp_data_blocks(const char* fn, const char *fl);
void comp_inodes(const char* fn, const char *fl);
void print_inode_data(const char* fn, const char *fl, Inode node);
void test_mkdir_bad_permissions();
void test_mkdir();
void test_mkdir_multiple_directories();

#endif