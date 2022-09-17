#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "xis.h"
#include "xcpu.h"

void xcpu_print( xcpu *c ) {
  int i;
  unsigned int op1;
  int op2;

  fprintf( stdout, "PC: %4.4x, State: %4.4x: Registers:\n", c->pc, c->state );
  for( i = 0; i < X_MAX_REGS; i++ ) {
    fprintf( stdout, " %4.4x", c->regs[i] );
  }
  fprintf( stdout, "\n" );

  op1 = c->memory[c->pc];
  op2 = c->memory[c->pc + 1];
  for( i = 0; i < I_NUM; i++ ) {
    if( x_instructions[i].code == c->memory[c->pc] ) {
      fprintf( stdout, "%s ", x_instructions[i].inst );
      break;
    }
  }

  switch( XIS_NUM_OPS( op1 ) ) {
  case 1:
    if( op1 & XIS_1_IMED ) {
      fprintf( stdout, "%d", op2 );
    } else {
      fprintf( stdout, "r%d", XIS_REG1( op2 ) );
    }
    break;
  case 2:
    fprintf( stdout, "r%d, r%d", XIS_REG1( op2 ), XIS_REG2( op2 ) );
    break;
  case XIS_EXTENDED:
    fprintf( stdout, "%u", (c->memory[c->pc + 2] << 8) | c->memory[c->pc + 3] );
    if( op1 & XIS_X_REG ) {
      fprintf( stdout, ", r%d", XIS_REG1( op2 ) );
    }
    break;
  }
  fprintf( stdout, "\n" );
}

#define MEM_IDX(x) ((x) % XIS_MEM_SIZE)

static void push( xcpu *c, unsigned short v ) {
  c->regs[X_STACK_REG] -= 2;
  c->memory[MEM_IDX(c->regs[X_STACK_REG])] = v >> 8;
  c->memory[MEM_IDX(c->regs[X_STACK_REG] + 1)] = v;
}

static unsigned short pop( xcpu *c ) {
  unsigned short v;
  v = ( c->memory[MEM_IDX(c->regs[X_STACK_REG])] << 8 ) |
        c->memory[MEM_IDX(c->regs[X_STACK_REG] + 1)];
  c->regs[X_STACK_REG] += 2;
  return v;
}

extern int xcpu_execute( xcpu *c ) {
  unsigned char op;
  char op2;
  int r1;
  int r2;
  unsigned short val;

  assert( c );

  op = c->memory[MEM_IDX(c->pc)];
  op2 = c->memory[MEM_IDX(c->pc + 1)];
  c->pc = MEM_IDX(c->pc + 2);
  r1 = ( op2 >> 4 ) & 0xf;
  r2 = op2 & 0xf;

  if( XIS_IS_EXT_OP( op ) ) {
    val = ( c->memory[MEM_IDX(c->pc)] << 8 ) | c->memory[MEM_IDX(c->pc + 1)];
    c->pc = MEM_IDX(c->pc + 2);
  }

  switch( op ) {
  case I_PUSH:
    push( c, c->regs[r1] );
    break;
  case I_POP:
    c->regs[r1] = pop( c );
    break;
  case I_MOV:
    c->regs[r2] = c->regs[r1];
    break;
  case I_LOAD:
    c->regs[r2] = ( c->memory[MEM_IDX(c->regs[r1])] << 8 ) |
                    c->memory[MEM_IDX(c->regs[r1] + 1)];
    break;
  case I_STOR:
    c->memory[MEM_IDX(c->regs[r2])] = c->regs[r1] >> 8;
    c->memory[MEM_IDX(c->regs[r2] + 1)] = c->regs[r1];
    break;
  case I_LOADB:
    c->regs[r2] = c->memory[MEM_IDX(c->regs[r1])];
    break;
  case I_STORB:
    c->memory[MEM_IDX(c->regs[r2])] = c->regs[r1];
    break;
  case I_JMPR:
    val = c->regs[r1];
  case I_JMP:
    c->pc = val;
    break;
  case I_CALLR:
    val = c->regs[r1];
  case I_CALL:
    push( c, c->pc );
    c->pc = val;
    break;
  case I_RET:
    c->pc = pop( c );
    break;
  case I_LOADI:
    c->regs[r1] = val;
    break;
  case I_ADD:
    c->regs[r2] += c->regs[r1];
    break;
  case I_SUB:
    c->regs[r2] -= c->regs[r1];
    break;
  case I_MUL:
    c->regs[r2] *= c->regs[r1];
    break;
  case I_DIV:
    c->regs[r2] /= c->regs[r1];
    break;
  case I_NEG:
    c->regs[r1] = -c->regs[r1];
    break;
  case I_AND:
    c->regs[r2] &= c->regs[r1];
    break;
  case I_OR:
    c->regs[r2] |= c->regs[r1];
    break;
  case I_XOR:
    c->regs[r2] ^= c->regs[r1];
    break;
  case I_NOT:
    c->regs[r1] = !c->regs[r1];
    break;
  case I_INC:
    c->regs[r1]++;
    break;
  case I_DEC:
    c->regs[r1]--;
    break;
  case I_SHL:
    c->regs[r2] <<= c->regs[r1];
    break;
  case I_SHR:
    c->regs[r2] >>= c->regs[r1];
    break;
  case I_TEST:
    if( c->regs[r2] & c->regs[r1] ) {
      c->state |= X_STATE_COND_FLAG;
    } else {
      c->state &= ~X_STATE_COND_FLAG;
    }
    break;
  case I_CMP:
    if( c->regs[r1] < c->regs[r2] ) {
      c->state |= X_STATE_COND_FLAG;
    } else {
      c->state &= ~X_STATE_COND_FLAG;
    }
    break;
  case I_EQU:
    if( c->regs[r1] == c->regs[r2] ) {
      c->state |= X_STATE_COND_FLAG;
    } else {
      c->state &= ~X_STATE_COND_FLAG;
    }
    break;
  case I_BR:
    if( c->state & X_STATE_COND_FLAG ) {
      c->pc += op2 - 2;
    }
    break;
  case I_JR:
    c->pc += op2 - 2;
    break;
  case I_CLD:
    c->state &= ~X_STATE_DEBUG_ON;
    break;
  case I_STD:
    c->state |= X_STATE_DEBUG_ON;
    break;
  case I_OUT:
    putchar( c->regs[r1] );
    break;
  case I_BAD:
  default:
    return 0;
  }

  if( c->state & X_STATE_DEBUG_ON ) {
    xcpu_print( c );
  }
  return 1;
}




