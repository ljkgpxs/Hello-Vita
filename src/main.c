#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/power.h>
#include <psp2/touch.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>

#define SCREEN_W	960
#define SCREEN_H	544

#define lerp(x, from_max, to_max) ((x*10.0 / (from_max*10.0) * (to_max*10.0)) / 10.0)
#ifdef __cplusplus
extern "C" {
	int scePowerSetArmClockFrequency(int freq);
}
#endif

int main(int argc, char *argv[])
{
	SceCtrlData pad;
	SceTouchData touch;
	SceUInt64 lastTime;
	SceInt64 sampleNum;

	SceFloat radi = 10.0f, lastLen = 0.0f, fps = 0.0f, tmp = 0.0f;
	SceInt32 freq = 333, x = SCREEN_W / 2.0, y = SCREEN_H / 2.0;

	vita2d_init();
	vita2d_set_clear_color(RGBA8(0, 0, 0, 255));
	vita2d_pgf *pgf = vita2d_load_default_pgf();

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);

	memset(&pad, 0, sizeof(pad));
	memset(&touch, 0, sizeof(touch));
	
	while(1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);

		if(pad.buttons & SCE_CTRL_START)
			break;
		else if(pad.buttons & SCE_CTRL_RTRIGGER)
			radi += 10;
		else if(pad.buttons & SCE_CTRL_LTRIGGER) {
			if(radi > 10.0f)
				radi -= 10;
		} else if(pad.buttons & SCE_CTRL_UP) {
			// cpu clock's range : 41-444
			freq += freq < 444 ? 1 : 0;
			scePowerSetArmClockFrequency(freq);
		} else if(pad.buttons & SCE_CTRL_DOWN) {
			freq -= freq > 41 ? 1 : 0;
			scePowerSetArmClockFrequency(freq);
		}

		if(touch.reportNum == 2) {
			// Get gesture to change the circle's radius
			tmp = (float)sqrt(pow(touch.report[0].x - touch.report[1].x, 2.0) + pow(touch.report[0].y - touch.report[1].y, 2.0));
			if(sampleNum == 1) {
				lastLen = tmp;
				lastTime = touch.timeStamp;
				sampleNum = 0;
			} else {
				if(touch.timeStamp - lastTime < 1E5) {
					// Time between two samples, and 1E6 is 1 second
					if(radi + (tmp - lastLen) / 2.0 > 0)
						radi += (tmp - lastLen) / 2.0;
					else
						radi = 0;
					lastLen = tmp;
					lastTime = touch.timeStamp;
				} else {
					lastLen = 0;
					sampleNum = 1;
				}
			x = lerp((abs(touch.report[0].x + touch.report[1].x) / 2.0), 1920, SCREEN_W);
			y = lerp((abs(touch.report[0].y + touch.report[1].y) / 2.0), 1088, SCREEN_H);
			}
		} else if(touch.reportNum == 3) break;

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_line(5, 5, SCREEN_W - 5, 5, RGBA8(255, 255, 255, 255));
		vita2d_draw_line(SCREEN_W - 5, 5, SCREEN_W - 5, SCREEN_H - 5, RGBA8(255, 255, 255, 255));
		vita2d_draw_line(SCREEN_W - 5, SCREEN_H - 5, 5, SCREEN_H - 5, RGBA8(255, 255, 255, 255));
		vita2d_draw_line(5, SCREEN_H - 5, 5, 5, RGBA8(255, 255, 255, 255));

		vita2d_draw_fill_circle(x, y, abs(radi), RGBA8(0, 255, 255, 255));
		vita2d_pgf_draw_textf(pgf, 25, 25, RGBA8(255, 255, 255, 255), 1.0f, \
			"Program By Ljkgpxs lx:%d ly:%d rx:%d ry:%d", pad.lx, pad.ly, pad.rx, pad.ry);
		vita2d_pgf_draw_textf(pgf, 25, 45, RGBA8(255, 255, 255, 255), 1.0f, " Current Arm Clock:%d Bus Clock:%d GPU Clock:%d", \
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
