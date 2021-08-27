/*-------------------------------------------------------------------------\
| main.h                                                                   |
|                                                                          |
| This file is part of smp_sim                                             |
|                                                                          |
| smp_sim is free software; you can redistribute it and/or modify          |
| it under the terms of the GNU General Public License as published by     |
| the Free Software Foundation; either version 2 of the License, or        |
| (at your option) any later version.                                      |
|                                                                          |
| smp_sim is distributed in the hope that it will be usedful,              |
| but WITHOUT ANY WARRANTY; without even the implied warranty of           |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             |
| GNU General Public License for more details.                             |
|                                                                          |
| You should have received a copy of the GNU General Public License        |
| along with smp_sim; if not, write to the Free Software                   |
| Foundation, Inc ., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA |
|-------------------------------------------------------------------------*/

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

struct INSTRUCTION
{
  int opcode;
  int operand[3];
};

int simulate (int n, int delay, FILE *test_file);
int get_instruction_block (struct INSTRUCTION *buffer, FILE *file);
int schedule_instructions (struct INSTRUCTION *buffer, int n);
int print_results (char *file, int n, int delay, int size, int time);

#endif /* !MAIN_H */
