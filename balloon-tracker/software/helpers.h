#pragma once

int get_free_memory();

void print_free_mem();

void log_init(const char file[], unsigned long size);

void chip_select_lora();
void chip_select_sd();