#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>



uint32_t PC = 0;
uint32_t I = 0;
uint8_t VX[16];
uint8_t *code;
uint32_t stack[16];
uint16_t sh = 0;
uint8_t memory[4096];

uint8_t fontSet[] = {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
uint8_t screen[64 * 32];

void flip_screen(uint8_t x, uint8_t y) {
    if (x>0 && x<64 && y>0 && y<32 ) {
        screen[x + y*64] = 1 - screen[x + y*64];
    }
}
void push_to_stack(uint16_t X) {
    stack[sh] = (uint32_t) X;
    sh +=1;
}
uint16_t pop_from_stack() {
    uint16_t X = (uint16_t) stack[sh];
    sh += -1;
    return X;
}


uint16_t fetch_current_instruction() {
    uint16_t n1 = (uint16_t) code[PC];
    uint16_t n2 = (uint16_t) code[PC + 1];
    uint16_t op = n1 << 8 | n2;
    return op;
}
uint8_t get_opcode(uint16_t op) {
    return (uint8_t) ((op & 0xf000) >> 12);
}

void clear_screen() {
    for (int i = 0; i < 64 * 32; i++) {
        screen[i] = 0;
    }
    printf("Clearing screen.\n");
}

void go_back() {
    // should put PC to current value
    PC = pop_from_stack();
    printf("Going back.\n");
}
void call(uint16_t op) {
    printf("call %x\n", op);
}

void goto_nnn(uint16_t NNN) {
    PC = NNN - 0x200;
    printf("goto_nnn %x\n", NNN);
}
void gosubroutine(uint16_t NNN) {
    push_to_stack(PC);
    PC = NNN - 0x200;
    printf("gotosub at %x\n", NNN);
}
void jmp_equ(uint8_t X, uint16_t NN) {
    if (VX[X] == NN) {
    PC += 2;
    }
    printf("jmp_equ if V%x == %x\n",X, NN);
}
void jmp_nequ(uint8_t X, uint16_t NN) {
    if (VX[X] != NN) {
        PC += 2;
    }
    printf("jmp_equ if V%x != %x\n",X, NN);
}
void jmp_equ_register(uint8_t X, uint8_t Y) {
    if (VX[X] == VX[Y]) {
        PC += 2;
    }
    printf("jmp_equ if V%x == V%x\n",X, Y);
}
void jmp_nequ_register(uint8_t X, uint8_t Y) {
    if (VX[X] != VX[Y]) {
        PC += 2;
    }
    printf("jmp_equ if V%x != V%x\n",X, Y);
}
void jmp(uint16_t NNN) {
    PC = VX[0] + NNN;

    printf("jmp to V0 + %x",NNN);
}
void reg_move(uint8_t X, uint16_t NN) {
    VX[X] = NN;
    printf("move  %x to V%x\n",NN, X);
}
void add(uint8_t X, uint16_t NN) {
    VX[X] += NN;
    printf("add  %x to V%x\n",NN, X);
}
void assign(uint8_t X, uint8_t Y) {
    VX[X] = VX[Y];
    printf("V%x = V%x\n",X, Y);
}
void assign_or(uint8_t X, uint8_t Y) {
    VX[X] |= VX[Y];
    printf("V%x |= V%x\n",X, Y);
}
void assign_and(uint8_t X, uint8_t Y) {
    VX[X] &= VX[Y];
    printf("V%x &= V%x\n",X, Y);
}
void assign_xor(uint8_t X, uint8_t Y) {
    VX[X] ^= VX[Y];
    printf("V%x ^= V%x\n",X, Y);
}
void register_add(uint8_t X, uint8_t Y) {
    VX[X] += VX[Y];
    printf("V%x += V%x\n",X, Y);
}
void register_sub(uint8_t X, uint8_t Y) {
    VX[X] -= VX[Y];
    printf("V%x -= V%x\n",X, Y);
}
void register_shiftr(uint8_t X) {
    VX[X] >>= 1 ;
    printf("V%x >> 1 store V%x & 0xf to VF\n",X, X);
}
void register_shiftl(uint8_t X) {
    VX[X] <<= 1 ;
    printf("V%x << 1 store VF to 1 overflmw\n",X);
}
void register_rsub(uint8_t X, uint8_t Y) {
    VX[X]  = VX[Y] - VX[X];
    printf("V%x = V%x - V%x\n",X, Y, X);
}
void set_address(uint16_t NNN) {
    I = NNN - 0x200;
    printf("set_address %x\n", NNN);
}
void set_rand(uint8_t X, uint16_t NNN) {
    VX[X] = rand() & NNN;
    printf("set_rand to V%x\n = rand() & %x", X, NNN);
}

void display(uint8_t X, uint8_t Y, uint8_t N) {
    uint8_t x = VX[X] & 63;
    uint8_t y = VX[Y] & 31;
    for (uint8_t i = 0; i < N; i++) {
        uint8_t v = memory[I + i];
        uint8_t ay = y + i;
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t ax = x + 7 - j;
            uint8_t flag = 1 << j;
            if ((v & flag) == flag) {
               // printf("screen %x %x\n", ax, ay);
                flip_screen(ax, ay);
            }
        }

    }

    printf("display sprite à (V%x, V%x) with height of %x - sprite at I\n", X,Y, N);
}

void key_pressed(uint8_t X) {
    printf("key pressed at V%x\n",X);
}
void key_release(uint8_t X) {
    printf("key release at V%x\n",X);
}

void set_delay(uint8_t X) {
    printf("set timer to V%x\n", X);
}

void get_key(uint8_t X) {
    printf("get key store at V%x\n", X);
}
void set_timer(uint8_t X) {
    printf("set timer at V%x\n", X);
}
void sound_timer(uint8_t X) {
    printf("sound timer at V%x\n", X);
}
void add_addr(uint8_t X) {
    I += VX[X];
    printf("I += V%x\n", X);
}
void set_sprite_addr(uint8_t X) {
    printf("set sprite addr at V%x\n", X);
}

void set_bcd(uint8_t X) {
    uint8_t x100 = X / 100;
    uint8_t x10 = (X  - x100 * 100) / 10;
    uint8_t x1 = X  - x100 * 100 - x10 *  10;
    memory[I] = x100;
    memory[I + 1] = x10;
    memory[I + 2] = x1;
    printf("set bcd V%x\n", X);
}
void store_registers(uint8_t X) {
    for (uint8_t i = 0; i < X; i++) {
        memory[I + i] = VX[i];
    }
    printf("store registers up to V%x at I\n", X);
}

void rec_registers(uint8_t X) {
    for (uint8_t i = 0; i < X; i++) {
        VX[i] = memory[I + i];
    }
    printf("rec registers up to V%x from I\n", X);
}

void decode_opcode(uint16_t op) {
    uint8_t opcode = (uint8_t) ((op & 0xf000) >> 12);
    uint8_t X = (uint8_t) ((op & 0x0f00) >> 8);
    uint8_t Y = (uint8_t) ((op & 0x00f0)>>4);
    uint8_t N = (uint8_t) ((op & 0x000f));
    uint8_t NN = (uint8_t) ((op & 0x00ff));
    uint16_t NNN = (uint16_t) (op & 0x0fff);
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
            reg_move(X, NN);
        }
        break;
        case 0x7 :  {
            add(X, NN);
        }
        break;
        case 8 : {
            uint8_t last_n = (uint8_t) (op & 0x000f);
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
            uint8_t last_n = (uint8_t) (op & 0x00ff);
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
            uint8_t last_n = (uint8_t) (op & 0x00ff);
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
    code = (uint8_t *) malloc(sizeof(uint8_t) * size);
    rewind(f);
    // and store in v
    fread(code, sizeof(uint8_t), size, f);
    printf("Read %ld bytes.\n", size);
    fclose(f);
    return size;
}

int main(void) {



    // open load code  file
    long size = load_file_and_code("../Particle Demo.ch8");
    memcpy(memory, code, size);
    // do the code
    int cpt = 0;
    while (1) {
        uint16_t op = fetch_current_instruction();
        PC += 2;
        decode_opcode(op);
        printf("state PC I stack V0 V1 \n %x %x %d %x %x\n",PC, I, sh, VX[0], VX[1]);
        cpt ++;
        if (cpt > 10000) {
            FILE * f = fopen("../img.txt", "w");

            for (int j = 0; j < 32; j++) {
                for (int i = 0; i < 64; i++) {
                if (screen[i + 64 *j ] == 1) {
                        fputc('*', f);
                        printf("*");
                    }
                    else {
                        fputc(' ', f);
                        printf(" ");
                    }
                }
                fputc('\n', f);
                printf("\n");
            }
            fclose(f);
            break;
        }

    }


    free(code);
    return 0;
}
