#ifndef __file_dialog_h__
#define __file_dialog_h__

#ifdef __cplusplus
extern "C" {
#endif

int SelectFileDialog(char *start_path, int mode, char *output_path);
int GetFilesList(char *start_path);

void FlipAndShow(void);
void Delay(int);
void DrawMessage(char *s, int nflip);
void DrawError(char *s, int nflip);
void Clrs(int color, int nflip);
void DrawWindow(int x, int y, int lx, int ly, int nflip, int c1, int c2, int c3, int c4, int c5);

#ifdef __cplusplus
}
#endif


#endif /*__file_dialog_h__*/