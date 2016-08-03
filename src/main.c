#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/power.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>

#define SCREEN_W	960
#define SCREEN_H	544

#define abs(x)		(x > 0 ? x : -x)

#ifdef __cplusplus
extern "C" {
	int scePowerSetArmClockFrequency(int freq);
}
#endif

int main(int argc, char *argv[])
{
	SceCtrlData pad;
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0, 0, 0, 255));
	vita2d_pgf *pgf = vita2d_load_default_pgf();

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

	float radi = 10.0f, fps = 0.0f;
	int freq = 400;

	memset(&pad, 0, sizeof(pad));
	
	while(1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if(pad.buttons & SCE_CTRL_START)
			break;
		else if(pad.buttons & SCE_CTRL_RTRIGGER)
			radi += 10;
		else if(pad.buttons & SCE_CTRL_LTRIGGER) {
			if(radi > 10.0f)
				radi -= 10;
		}
		else if(pad.buttons & SCE_CTRL_UP) {
			freq += 1;
			scePowerSetArmClockFrequency(freq);
		}
		else if(pad.buttons & SCE_CTRL_DOWN) {
			freq -= freq > 41 ? 1 : 0;
			scePowerSetArmClockFrequency(freq);
		}

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_line(5, 5, SCREEN_W - 5, 5, RGBA8(255, 255, 255, 255));
		vita2d_draw_line(SCREEN_W - 5, 5, SCREEN_W - 5, SCREEN_H - 5, RGBA8(255, 255, 255, 255));
		vita2d_draw_line(SCREEN_W - 5, SCREEN_H - 5, 5, SCREEN_H - 5, RGBA8(255, 255, 255, 255));
		vita2d_draw_line(5, SCREEN_H - 5, 5, 5, RGBA8(255, 255, 255, 255));

		vita2d_draw_fill_circle(SCREEN_W / 2, SCREEN_H / 2, abs(radi), RGBA8(0, 255, 255, 255));
		vita2d_pgf_draw_textf(pgf, 25, 25, RGBA8(255, 255, 255, 255), 1.0f, \
			"Program By Ljkgpxs lx:%d ly:%d rx:%d ry:%d", pad.lx, pad.ly, pad.rx, pad.ry);
		vita2d_pgf_draw_textf(pgf, 25, 45, RGBA8(255, 255, 255, 255), 1.0f, " Current CPU clock:%d Bus Clock:%d GPU clock:%d", \
			scePowerGetArmClockFrequency(), scePowerGetBusClockFrequency(), scePowerGetGpuClockFrequency());
		
		sceDisplayGetRefreshRate(&fps);
		vita2d_pgf_draw_textf(pgf, SCREEN_W - 80, 25, RGBA8(255, 255, 255, 255), 0.7f, "FPS %.2f", fps);

		vita2d_end_drawing();
		vita2d_swap_buffers();
		sceDisplayWaitVblankStart();
	}

	vita2d_fini();
	vita2d_free_pgf(pgf);
	sceKernelExitProcess(0);
	return 0;
}
