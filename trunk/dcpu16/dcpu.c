/* DCPU-16 emulator, as seen on 0x10c 
 * ISA specification is copyright 2012, Mojang Ab
 * This emulator is copyrighti 2012 Kirn Gill <segin2005@gmail.com>
 * See COPYING for license text.
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char *argv[]) 
{

	/* the registers */ 
	uint16_t reg_a, reg_b, reg_c, reg_x, reg_y, reg_z, reg_i,
		 reg_j, reg_pc, reg_sp, reg_o;

	/* main system memory */
	uint16_t mem[0x10000];
	
	/* CPU internals */
	uint16_t op_code, op_a, op_b;

	/* fuck you, goto is perfectly fine if used correctly */
init:
	reg_a = reg_b = reg_c = reg_x = reg_y = reg_z = reg_i = reg_j = reg_pc 
        = reg_o = 0;
	reg_sp = 0xffff;

	/* CPU main loop */
	while(1) { 
	


	}
}
