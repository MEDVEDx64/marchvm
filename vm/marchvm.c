/* ========================================

  MarchVM (Medvedish Architecture
    Virtual Machine)
  medvedx64 2013
  comes under the terms of the MIT license

======================================== */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct marchCPU { // Registers

    uint16_t CC; // Current offset in code
    uint16_t LL; // Collector

    uint16_t CR; // Instruction register
    uint16_t A1; // Args
    uint16_t A2;
    uint16_t A3;

    uint16_t D1; // Data registers
    uint16_t D2;
    uint16_t D3;
    uint16_t D4;

} marchCPU;

/*
#define marchCPU_D1A (uint8_t)(D1>>8)
#define marchCPU_D1B (uint8_t)D1

#define marchCPU_D2A (uint8_t)(D2>>8)
#define marchCPU_D2B (uint8_t)D2

#define marchCPU_D3A (uint8_t)(D3>>8)
#define marchCPU_D3B (uint8_t)D3

#define marchCPU_D4A (uint8_t)(D4>>8)
#define marchCPU_D4B (uint8_t)D4
*/

#define MAX_NUM (65536-64)

enum {
    marchcmd_np, // nothing
    marchcmd_ex, // interrupt

    marchcmd_cp = 128, // copy

    marchcmd_gt = 256, // Get byte from memory
    marchcmd_pt = 288, // Put byte into memory

    marchcmd_ad = 1024,
    marchcmd_sb = 1536,
    marchcmd_mp = 2048,
    marchcmd_dv = 2560,

    marchcmd_an = 5192, // and
    marchcmd_or = 5320, // or
    marchcmd_tr = 5448, // move a1 right by 1 bit
    marchcmd_tl = 5576, // move a1 left by 1 bit

    marchcmd_ju = 10240, // jump to a2 if a1 != 0
    marchcmd_ig = 10272, // if a1 > a2 then ll = 2;
                            // if a1 == a2 then ll = 1;
                            // if a1 < a2 then ll = 0.

    marchcmd_gg = MAX_NUM-1 // halt

};

enum {
    marchreg_cc = 65534,
    marchreg_ll = 65532,
    marchreg_cr = 65530,

    marchreg_a1 = 65529,
    marchreg_a2 = 65528,
    marchreg_a3 = 65527,

    marchreg_d1  = 65514,
    marchreg_d1a = 65513,
    marchreg_d1b = 65512,

    marchreg_d2  = 65510,
    marchreg_d2a = 65509,
    marchreg_d2b = 65508,

    marchreg_d3  = 65506,
    marchreg_d3a = 65505,
    marchreg_d3b = 65504,

    marchreg_d4  = 65502,
    marchreg_d4a = 65501,
    marchreg_d4b = 65500
};


enum {
    marchexarg_refresh_screen = 2
};

// Text screen widht/height
#define SW 86
#define SH 48
#define SYM_SIZE 10 // multiplier to pixels

// Memory
void *mem = NULL;
#define MEM_SIZE 65536 // 64k
// Video and keyboard buffers
void *kbd_mem = NULL;
void *video_mem = NULL;
#define KBD_MEM_OFFSET 128
#define KBD_MEM_SIZE 384
#define VIDEO_MEM_OFFSET 512
#define VIDEO_MEM_SIZE (SW*SH*2) // 16 bits per symbol
uint16_t *ll_reg_ptr = (uint16_t*)&marchCPU.LL;

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

int startVideo() {
    if(SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Fatal: can't init SDL: %s\n", SDL_GetError());
        return -1;
    }

    if(!SDL_SetVideoMode(SW*SYM_SIZE, SH*SYM_SIZE, 0, SDL_HWSURFACE|SDL_DOUBLEBUF)) {
        fprintf(stderr, "Fatal: can't create screen: %s\n", SDL_GetError());
        return -1;
    }

    fprintf(stderr, "Video subsystem has arrived.\n");
    return 0;
}

void refreshVideo() {
    register int x = SW;
    register int y = SH;
    SDL_Surface *scr = SDL_GetVideoSurface();

    register int xpos_buf, ypos_buf = 0;
    uint16_t sym = 0;

    while(y) {
        x = SW;
        while(x) {
            sym = (((uint16_t*)video_mem)[((y-1)*SW+(x-1))]); // finding symbol in the video memory
            xpos_buf = (x-1)*SYM_SIZE;
            ypos_buf = (y-1)*SYM_SIZE;

            boxColor(scr, xpos_buf, ypos_buf, xpos_buf+SYM_SIZE, ypos_buf+SYM_SIZE,
                ((sym&0x400)<<21)|((sym&0x200)<<14)|((sym&0x100)<<7)|0xff); // obtaining a color
            characterColor(scr, xpos_buf+2, ypos_buf+2, (char)sym,
                ((sym&0x2000)<<18)|((sym&0x1000)<<11)|((sym&0x800)<<4)|0xff);

            --x;
        }

        --y;
    }

    SDL_Flip(scr);

}

void initVM() {
    mem = malloc(MEM_SIZE);
    memset(mem, 0, MEM_SIZE);

    kbd_mem = mem+KBD_MEM_OFFSET;
    video_mem = mem+VIDEO_MEM_OFFSET;

    marchCPU.CC = 0;
}

FILE *code_file = NULL;
int isHalted = 0;

void *regAccess__(uint16_t arg) {
    switch(arg) {
        case marchreg_d4b:  return 1+(uint8_t*)&marchCPU.D4;
        case marchreg_d4a:  return (uint8_t*)&marchCPU.D4;
        case marchreg_d4:   return &marchCPU.D4;

        case marchreg_d3b:  return 1+(uint8_t*)&marchCPU.D3;
        case marchreg_d3a:  return (uint8_t*)&marchCPU.D3;
        case marchreg_d3:   return &marchCPU.D3;

        case marchreg_d2b:  return 1+(uint8_t*)&marchCPU.D2;
        case marchreg_d2a:  return (uint8_t*)&marchCPU.D2;
        case marchreg_d2:   return &marchCPU.D2;

        case marchreg_d1b:  return 1+(uint8_t*)&marchCPU.D1;
        case marchreg_d1a:  return (uint8_t*)&marchCPU.D1;
        case marchreg_d1:   return &marchCPU.D1;

        case marchreg_a1:   return &marchCPU.A1;
        case marchreg_a2:   return &marchCPU.A2;
        case marchreg_a3:   return &marchCPU.A3;

        case marchreg_cc:   return &marchCPU.CC;
        case marchreg_cr:   return &marchCPU.CR;
        case marchreg_ll:   return &marchCPU.LL;

        default: return NULL;
    }
}

// Keyboard states
#define K_NULL          0
#define K_DOWN          1
#define K_UP            2
#define K_PRESSED       3

void kbdCtl(void *kbdbuf, uint16_t buflen) {
    register int i = 0;
    while(i < buflen)
    {
        if(((uint8_t*)kbdbuf)[i] == K_DOWN) ((uint8_t*)kbdbuf)[i] = K_PRESSED;
        if(((uint8_t*)kbdbuf)[i] == K_UP)   ((uint8_t*)kbdbuf)[i] = K_NULL;

        ++i;
    }

    SDL_Event ev;
    while(SDL_PollEvent(&ev))
    {
        if(ev.type == SDL_QUIT)         isHalted = 1;
        if(ev.type == SDL_KEYDOWN)      ((uint8_t*)kbdbuf)[ev.key.keysym.sym] = K_DOWN;
        if(ev.type == SDL_KEYUP)        ((uint8_t*)kbdbuf)[ev.key.keysym.sym] = K_UP;
    }
}

#define RACCESS(arg) ((arg) > MAX_NUM? *(uint16_t*)regAccess__(arg): (arg))
#define WACCESS(arg) ((arg) > MAX_NUM? (uint16_t*)regAccess__(arg): ll_reg_ptr)
uint64_t iterations = 0;

void VMLoop() {

    uint64_t buffer = 0;
    if(fread(&buffer, 8, 1, code_file) != 1) {
        fprintf(stderr, "Read error or reached end of file, halting.\n");
        isHalted = 1;
        return;
    }

    // Pushing it into registers
    marchCPU.CR = (uint16_t)buffer;
    marchCPU.A1 = (uint16_t)(buffer>>16);
    marchCPU.A2 = (uint16_t)(buffer>>32);
    marchCPU.A3 = (uint16_t)(buffer>>48);

    uint16_t *wa = NULL;

    switch(marchCPU.CR) {

        case marchcmd_np: break;
        case marchcmd_gg: isHalted = 1; break;
        case marchcmd_ex: {
            switch(marchCPU.A1) {
                case marchexarg_refresh_screen:
                    refreshVideo();
                    break;
            }
        }
        case marchcmd_ad: marchCPU.LL = RACCESS(marchCPU.A1)+RACCESS(marchCPU.A2); break;
        case marchcmd_sb: marchCPU.LL = RACCESS(marchCPU.A1)-RACCESS(marchCPU.A2); break;
        case marchcmd_mp: marchCPU.LL = RACCESS(marchCPU.A1)*RACCESS(marchCPU.A2); break;
        case marchcmd_dv: marchCPU.LL = RACCESS(marchCPU.A1)/RACCESS(marchCPU.A2); break;

        case marchcmd_cp: wa = WACCESS(marchCPU.A2);
            *wa = RACCESS(marchCPU.A1); break;
        case marchcmd_gt: wa = WACCESS(marchCPU.LL);
            *wa = ((uint8_t*)mem)[RACCESS(marchCPU.A1)]; break;
        case marchcmd_pt: ((uint8_t*)mem)[RACCESS(marchCPU.A1)] = (uint8_t)RACCESS(marchCPU.A2); break;

        case marchcmd_an: marchCPU.LL = RACCESS(marchCPU.A1)&RACCESS(marchCPU.A2); break;
        case marchcmd_or: marchCPU.LL = RACCESS(marchCPU.A1)|RACCESS(marchCPU.A2); break;
        case marchcmd_tr: marchCPU.LL = RACCESS(marchCPU.A1)>>1; break;
        case marchcmd_tl: marchCPU.LL = RACCESS(marchCPU.A1)<<1; break;
        case marchcmd_ju: if(RACCESS(marchCPU.A1)) marchCPU.CC = RACCESS(marchCPU.A2); break;
        case marchcmd_ig:
            if(RACCESS(marchCPU.A1) > RACCESS(marchCPU.A2)) marchCPU.LL = 2;
            else if(RACCESS(marchCPU.A1) == RACCESS(marchCPU.A2)) marchCPU.LL = 1;
            else if (RACCESS(marchCPU.A1) < RACCESS(marchCPU.A2)) marchCPU.LL = 0; break;

        default: fprintf(stderr, "Caught unknown instruction\n"); isHalted = 1;

    }

    ++marchCPU.CC;
    fseek(code_file, marchCPU.CC*8, SEEK_SET);
    ++iterations;

}

void destroyMV() {
    free(mem);
    video_mem = NULL;
    kbd_mem = NULL;
}

int main(int c, char *v[]) {

    if(c > 1) {
        code_file = fopen(v[1], "rb");
        if(!code_file) {
            fprintf(stderr, "Failed to open the file (%s).\n", v[1]);
            return -1;
        }

    } else {
        fprintf(stderr, "usage: %s rom_image\n", v[0]);
        return 0;
    }

    /*if(!code_file)
            code_file = stdin;*/

    fprintf(stderr, "Starting MarchVM\n");
    if(startVideo()) {
        fprintf(stderr, "Exiting with error state.\n");
        return -1;
    }

    initVM();
    SDL_WM_SetCaption("MarchVM", 0);

    while(1) {
        kbdCtl(kbd_mem, KBD_MEM_SIZE);
        VMLoop();
        if(isHalted)
            break;
    }

    fprintf(stderr, "System halted (%llu cycles)\n"
        //"Writing memory dump to stdout.\n"
        , iterations);
    //fwrite(mem, MEM_SIZE, 1, stdout);
    SDL_WM_SetCaption("MarchVM (halted)", 0);
    
    while(1) {
        SDL_Event e;
        if(SDL_PollEvent(&e)) {
            if(e.type == SDL_KEYDOWN) {
                if(e.key.keysym.sym == SDLK_w) {
                    fprintf(stderr, "Writing memory dump to stdout\n");
                    fwrite(mem, MEM_SIZE, 1, stdout);
                }
            }
            else if(e.type == SDL_QUIT)
                break;
        }
        else
            SDL_Delay(64);

    }

    fprintf(stderr, "Power down.\n");
    destroyMV();
    SDL_Quit();
    return 0;
}
