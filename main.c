#include <stdio.h>
#include <stdlib.h>

uint32_t PC = 0;
uint32_t I = 0;
u_int8_t VX[16];
u_int8_t *code;
u_int32_t stack[16];
u_int16_t sh = 0;

void push_to_stack(u_int16_t X) {
    stack[sh] = (u_int32_t) X;
    sh +=1;
}
u_int16_t pop_from_stack() {
    u_int16_t X = (u_int16_t) stack[sh];
    sh += -1;
    return X;
}


u_int16_t fetch_instruction(uint32_t PC, u_int8_t * code) {
    u_int16_t n1 = (u_int16_t) code[PC];
    u_int16_t n2 = (u_int16_t) code[PC + 1];
    u_int16_t op = n1 << 8 | n2;
    return op;
}
u_int8_t get_opcode(u_int16_t op) {
    return (u_int8_t) ((op & 0xf000) >> 12);
}

void clear_screen() {
    printf("Clearing screen.\n");
}

void go_back() {
    // should put PC to current value
    PC = pop_from_stack();
    printf("Going back.\n");
}
void call(u_int16_t op) {
    printf("call %x\n", op);
}

void goto_nnn(u_int16_t NNN) {
    PC = NNN;
    printf("goto_nnn %x\n", NNN);
}
void gosubroutine(u_int16_t NNN) {
    push_to_stack(PC);
    PC = NNN;
    printf("gotosub at %x\n", NNN);
}
void jmp_equ(u_int8_t X, u_int16_t NN) {
    printf("jmp_equ if V%x == %x\n",X, NN);
}
void jmp_nequ(u_int8_t X, u_int16_t NN) {
    printf("jmp_equ if V%x != %x\n",X, NN);
}
void jmp_equ_register(u_int8_t X, u_int8_t Y) {
    printf("jmp_equ if V%x == V%x\n",X, Y);
}
void jmp_nequ_register(u_int8_t X, u_int8_t Y) {
    printf("jmp_equ if V%x != V%x\n",X, Y);
}
void jmp(u_int16_t NNN) {
    printf("jmp to V0 + %x",NNN);
}
void move(u_int8_t X, u_int16_t NN) {
    VX[X] = NN;
    printf("move  %x to V%x\n",NN, X);
}
void add(u_int8_t X, u_int16_t NN) {
    VX[X] += NN;
    printf("add  %x to V%x\n",NN, X);
}
void assign(u_int8_t X, u_int8_t Y) {
    VX[X] = VX[Y];
    printf("V%x = V%x\n",X, Y);
}
void assign_or(u_int8_t X, u_int8_t Y) {
    VX[X] |= VX[Y];
    printf("V%x |= V%x\n",X, Y);
}
void assign_and(u_int8_t X, u_int8_t Y) {
    VX[X] &= VX[Y];
    printf("V%x &= V%x\n",X, Y);
}
void assign_xor(u_int8_t X, u_int8_t Y) {
    VX[X] ^= VX[Y];
    printf("V%x ^= V%x\n",X, Y);
}
void register_add(u_int8_t X, u_int8_t Y) {
    VX[X] += VX[Y];
    printf("V%x += V%x\n",X, Y);
}
void register_sub(u_int8_t X, u_int8_t Y) {
    VX[X] -= VX[Y];
    printf("V%x -= V%x\n",X, Y);
}
void register_shiftr(u_int8_t X) {
    VX[X] >>= 1 ;
    printf("V%x >> 1 store V%x & 0xf to VF\n",X, X);
}
void register_shiftl(u_int8_t X) {
    VX[X] <<= 1 ;
    printf("V%x << 1 store VF to 1 overflmw\n",X);
}
void register_rsub(u_int8_t X, u_int8_t Y) {
    VX[X]  = VX[Y] - VX[X];
    printf("V%x = V%x - V%x\n",X, Y, X);
}
void set_address(u_int16_t NNN) {
    I += NNN;
    printf("set_address %x\n", NNN);
}
void set_rand(u_int8_t X, u_int16_t NNN) {
    VX[X] = rand() & NNN;
    printf("set_rand to V%x\n = rand() & %x", X, NNN);
}

void display(u_int8_t X, u_int8_t Y, u_int8_t N) {
    printf("display sprite à (V%x, V%x) with height of %x - sprite at I\n", X,Y, N);
}

void key_pressed(u_int8_t X) {
    printf("key pressed at V%x\n",X);
}
void key_release(u_int8_t X) {
    printf("key release at V%x\n",X);
}

void set_delay(u_int8_t X) {
    printf("set timer to V%x\n", X);
}

void get_key(u_int8_t X) {
    printf("get key store at V%x\n", X);
}
void set_timer(u_int8_t X) {
    printf("set timer at V%x\n", X);
}
void sound_timer(u_int8_t X) {
    printf("sound timer at V%x\n", X);
}
void add_addr(u_int8_t X) {
    I += VX[X];
    printf("I += V%x\n", X);
}
void set_sprite_addr(u_int8_t X) {
    printf("set sprite addr at V%x\n", X);
}

void set_bcd(u_int8_t X) {
    printf("set bcd V%x\n", X);
}
void store_registers(u_int8_t X) {
    printf("store registers up to V%x at I\n", X);
}

void rec_registers(u_int8_t X) {
    printf("rec registers up to V%x from I\n", X);
}

void decode_opcode(u_int16_t op) {
    u_int8_t opcode = (u_int8_t) ((op & 0xf000) >> 12);
    u_int8_t X = (u_int8_t) ((op & 0x0f00) >> 8);
    u_int8_t Y = (u_int8_t) ((op & 0x00f0)>>4);
    u_int8_t N = (u_int8_t) ((op & 0x000f));
    u_int8_t NN = (u_int8_t) ((op & 0x00ff));
    u_int16_t NNN = (u_int16_t) (op & 0x0fff);
    printf("%x %x %x %x %x %x %x\n",op, opcode,X,Y,N,NN,NNN);
    switch(opcode) {
        case 0 : {
            if (op == 0x00e0) {
                clear_screen();
            }
            else if (op == 0x00ee) {
                go_back();
            }
            else {
                call(NNN);
            }
        };break;

        case 1 : {
            goto_nnn(NNN);
        }
        break;
        case 2 : {
            gosubroutine(NNN);
        } break;

        case 3 : {
            jmp_equ(X, NN);
        }
        break;
        case 4 : {
            jmp_nequ(X, NN);
        }
        break;
        case 5 :  {
            jmp_equ_register(X, Y);
        }
        break;
        case 0x6 : {
            move(X, NN);
        }
        break;
        case 0x7 :  {
            add(X, NN);
        }
        break;
        case 8 : {
            u_int8_t last_n = (u_int8_t) (op & 0x000f);
            switch(last_n) {
                case 0 : {
                    assign(X,Y);
                }
                break;
                case 1 : {
                    assign_or(X,Y);
                }
                break;
                case 2 : {
                    assign_and(X,Y);
                }
                break;
                case 3 : {
                    assign_xor(X,Y);
                }
                break;
                case 4 : {
                    register_add(X,Y);
                }
                break;
                case 5 : {
                   register_sub(X,Y);
                }
                break;
                case 6 : {
                    register_shiftr(X);
                }
                break;
                case 7 : {
                    register_rsub(X,Y);
                }
                break;
                case 0xe : {
                    register_shiftl(X);
                }
                break;
                default : {
                    printf("unknown opcode %4x.\n", op);
                }
                break;
            }
        }
        break;
        case 9 : {
            jmp_nequ_register(X,Y);
        }
        break;
        case 0xa : {
            set_address(NNN);
        }
        break;

        case 0xb : {
            jmp(NNN);
        };break;

        case 0xc : {
            set_rand(X, NN);
        };break;
        case 0xd : {
            display(X, Y, N);
        }
        break;
        case 0xe : {
            u_int8_t last_n = (u_int8_t) (op & 0x00ff);
            if (last_n == 0x9e) {
                key_pressed(X);
            }
            else if (last_n == 0xa1) {
                key_release(X);
            }
            else {
                printf("unknown opcode %4x.\n", op);
            }
        }
        break;
        case 0xf : {
            u_int8_t last_n = (u_int8_t) (op & 0x00ff);
            switch (last_n) {
                case 0x07: {
                    set_delay(X);
                }
                break;
                case 0x0a: {
                    get_key(X);
                }
                break;
                case 15: {
                    set_timer(X);
                }
                break;
                case 18: {
                    sound_timer(X);
                }
                break;
                case 0x1e: {
                    add_addr(X);
                }
                break;
                case 0x29: {
                    set_sprite_addr(X);
                }
                break;
                case 0x33: {
                    set_bcd(X);
                }
                break;
                case 0x55: {
                    store_registers(X);
                }
                break;
                case 0x65: {
                   rec_registers(X);
                }
                break;
                default : {
                    printf("unknown opcode %4x.\n", op);
                }
            }
        }
        break;

        default : printf("unknown opcode %4x.\n", op);break;
    }
}

long load_file_and_code(char * filename) {
    FILE * f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Error opening file.\n");
        exit(2);
    }
    // find file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    printf("File size is %ld bytes.\n", size);
    // reserve memory for whole file
    code = (u_int8_t *) malloc(sizeof(u_int8_t) * size);
    rewind(f);
    // and store in v
    fread(code, sizeof(u_int8_t), size, f);
    printf("Read %ld bytes.\n", size);
    fclose(f);
    return size;
}

int main(void) {



    // open load code  file
    long size = load_file_and_code("../IBM Logo.ch8");

    // do the code
    while (1) {
        u_int16_t op = fetch_instruction(PC, code);
        PC += 2;
        decode_opcode(op);
        printf("state PC I stack V0 V1 \n %x %x %d %x %x\n",PC, I, sh, VX[0], VX[1]);
    }


    free(code);
    return 0;
}
