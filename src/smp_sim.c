/*-------------------------------------------------------------------------\
| main.c                                                                   |
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "smp_sim.h"

int
main (int argc, char **argv)
{
  /*
    Usage: smp_sim <test file 1> <test file 2> ...

    Output is sent to STDOUT
  */
  if (argc < 2)
    {
      fprintf (stderr, "Usage: smp_sim <test file 1> <test file 2> ...\n");
      return (1);
    }

  int n[] = {1, 4, 8, 16, 32};
  int delay[] = {2, 8};
  int size;

  int i, j, k;
  FILE *test_file = NULL;
  for (j = 0; j < 2; j++)
    {
      printf ("FETCH DELAY: %d\n", delay[j]);
      printf ("SIZE  N CYCLES UTILIZATION\n");
      printf ("--------------------------\n");
      for (k = 1; k < argc; k++)
        {
          size = atoi (argv[k]);
          for (i = 0; i < 5; i++)
            {
              test_file = fopen (argv[k], "r");
              print_results (argv[k], n[i], delay[j], size, simulate (n[i], delay[j], test_file));
              fclose (test_file);
            }
        }
      printf ("\n");
    }

  return (0);
}

int
simulate (int n, int delay, FILE *test_file)
{
  int time = 0;   /* Time to execute program */
  int no_more_code = 0;
  struct INSTRUCTION schedule_buffer[32];
  int i;

  for (i = 0; i < 32; i++)
    {
      schedule_buffer[i].opcode = 0;
      schedule_buffer[i].operand[0] = 0;
      schedule_buffer[i].operand[1] = 0;
      schedule_buffer[i].operand[2] = 0;
    }

  /* Fill the buffer */
  get_instruction_block (schedule_buffer, test_file);
  time += delay;
  if (get_instruction_block (schedule_buffer, test_file) == 0)
    time += delay;

  /* Loop until we're out of code */
  for (;;)
    {
      /* Quit if we have emptied the buffer
         and there is no more code to fetch */
      if (schedule_buffer[0].opcode == 0 &&
          no_more_code == 1)
        break;

      /* See if we need to fetch a block of instructions */
      if ((schedule_buffer[0].opcode == 0 ||
          schedule_buffer[16].opcode == 0) &&
          no_more_code == 0)
        {
          /* Fetch a block */
          no_more_code = get_instruction_block (schedule_buffer, test_file);

          /* We fetched some code, which cost some time */
          if (!no_more_code)
            time += delay;
        }

      /* Schedule (and delete) instructions */
      schedule_instructions (schedule_buffer, n);

      /* Execute one cycle */
      time++;
    }

  return (time);
}

int
get_instruction_block (struct INSTRUCTION *buffer, FILE *file)
{
  int lower = 0;
  int upper = 15;

  if (buffer[0].opcode != 0)
    {
      lower = 16;
      upper = 31;
    }

  int i;
  for (i = lower; i < upper + 1; i++)
    {
      /* Read one instruction */
      char line[20];
      if (fgets (line, 20, file) == NULL)
        return (1);

      /* Set a dummy opcode */
      buffer[i].opcode = 1;

      /* Get the operand arguments */
      char *token, *cp;
      cp = strdup (line);
      token = strtok (cp, " ");
      token = strtok (NULL, "r");
      buffer[i].operand[0] = atoi (token);
      token = strtok (NULL, "r");
      buffer[i].operand[1] = atoi (token);
      token = strtok (NULL, "r");
      buffer[i].operand[2] = atoi (token);
    }

  return (0);
}

int
schedule_instructions (struct INSTRUCTION *buffer, int n)
{
  //  printf ("schedule\n");
  
  int scheduled = 0;
  /* 0 => register not used
     1 => register used as a destination
     2 => register used as an argument */
  int WRITE = 0x01;
  int READ  = 0x10;
  int register_usage[51];

  int i;
  for (i = 0; i < 51; i++)
    register_usage[i] = 0;

  /* Loop through the whole buffer */
  for (i = 0; i < 32; i++)
    {
      /* Skip deleted instructions (just to be safe)*/
      if (buffer[i].opcode == 0)
        continue;

      /* Do not schedule instructions that read registers
         that are marked for writing this round.
         Do not schedule instructions that write to registers
         that are marked for reading this round */
      if ((register_usage[ buffer[i].operand[1] ] & WRITE) == 0x01 ||
          (register_usage[ buffer[i].operand[2] ] & WRITE) == 0x01 ||
          (register_usage[ buffer[i].operand[0] ] & READ) == 0x10)
        {
          /* Mark the destination register as in use so
             that instructions that need it can't
             be scheduled */
          register_usage[ buffer[i].operand[0] ] |= WRITE;
          /* Mark the registers it reads so that we can't
             schedule an instruction that writes to them */
          register_usage[ buffer[i].operand[1] ] |= READ;
          register_usage[ buffer[i].operand[2] ] |= READ;
          continue;
        }

      /* Mark the destination register as in use so
         that instructions that need it can't
         be scheduled */
      register_usage[ buffer[i].operand[0] ] |= WRITE;
      /* Mark the registers it reads so that we can't
         schedule an instruction that writes to them */
      register_usage[ buffer[i].operand[1] ] |= READ;
      register_usage[ buffer[i].operand[2] ] |= READ;

      /* Delete instruction */
      buffer[i].opcode = 0;

      /* We have used up one more processor */
      scheduled++;

      //      printf ("\t%d\n", buffer[i].operand[0]);

      /* Return when we don't have any more
         processors to pass instructions to */
      if (scheduled == n)
        break;
    }

  /* Consolidate the buffer */
  struct INSTRUCTION temp[32];
  for (i = 0; i < 32; i++)
    {
      temp[i].opcode = 0;
      temp[i].operand[0] = 0;
      temp[i].operand[1] = 0;
      temp[i].operand[2] = 0;
    }

  int j;
  for (i = 0, j = 0; i < 32; i++)
    {
      if (buffer[i].opcode != 0)
        temp[j++] = buffer[i];
    }

  for (i = 0; i < 32; i++)
    buffer[i] = temp[i];

  return (0);
}

int
print_results (char *file, int n, int delay, int size, int time)
{
  printf ("%4d %2d %6d %11.2f\n", size, n, time, (double) size / (time * n));
  return (1);
}
