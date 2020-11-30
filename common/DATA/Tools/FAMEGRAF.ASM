	IDEAL
	JUMPS
	LOCALS
	SMART
	P386

	DOSSEG
	MODEL	LARGE
;Процедуры и функции
        PUBLIC  @Vga256$qv		;установка граф.режима
        PUBLIC  @SetScreen$qnv	;установка скрытого экрана в памяти
        PUBLIC  @NormalScreen$qv	;установка обучн экрана
        PUBLIC  @ScreenCopy$qnvnv     ;копирование 2х экранов
        PUBLIC  @CopyToScreen$qnv   ;копирование на экран с ожиданием
        PUBLIC  @Cls$qzc	;очистка тек экрана
        PUBLIC  @WVR$qv            ;ожидание отр луча.???
        PUBLIC  @SetColor$qzc	;уст.текущий цвет
        PUBLIC  @PutRGB$qzczczczc ;установить один цвет палитры
        PUBLIC  @GetPalette$qnzc     ;взять текущую палитру
	PUBLIC	@GetCharWidth$qi	;ширина символа
        PUBLIC  @PutFont$qnzc        ;установить текущий шрифт (8*8)
        PUBLIC  @DisplayChar$qiiizczc    ;вывести символ текущим шрифтом
        PUBLIC  @Clip$qiiii		;установка окна обрезания
        PUBLIC  @PutPixel$qii       ;нарисовать точку тек.цветом
        PUBLIC  @GetPixel$qii       ;считать точку с экрана
        PUBLIC  @Bar$qiiii		;прямоугольник
;	PUBLIC  @Rectangle$qiiii	;рамка
        PUBLIC  @PutImg$qiiiinzc         ;вывод блока на экран
        PUBLIC  @GetImg$qiiiinzc         ;считать блок с экрана
        PUBLIC  @PutMas$qiiiinzc         ;вывод блока с прозрачным цветом на экран
        PUBLIC  @PutMasr$qiiiinzc        ;вывод блока с прозрачным цветом на экран r
        PUBLIC  @PutBlink$qiiiinzc	;вывод мерцающего силуэта
        PUBLIC  @PutBlinkr$qiiiinzc	;вывод мерцающего силуэта r

        PUBLIC  @PutSImg$qiiiiiiiinzc	;вывод с обрезкой спрайта на экран
        PUBLIC  @PutSMas$qiiiiiiiinzc	;вывод с обрезкой спрайта на экран
        PUBLIC  @PutSMasr$qiiiiiiiinzc	;вывод с обрезкой спрайта на экран
        PUBLIC  @PutSBlink$qiiiiiiiinzc	;вывод с обрезкой мерцающего силуэта
        PUBLIC  @PutSBlinkr$qiiiiiiiinzc	;вывод с обрезкой мерцающего силуэта r

	;PUBLIC	@CopyBlock$qiiiinviinv	;копирование прямоугольного блока памяти
	PUBLIC	_CopyBlock	;копирование прямоугольного блока памяти
	PUBLIC	@CopyBlock0$qnv	;копирование прямоуг блока памяти на экр
        PUBLIC  @PutImg16$qiinzc	;вывод блока 16*16 на экран
        PUBLIC  @PutMas16$qiinzc	;вывод блока 16*16 с прозрачным цветом на экр
        PUBLIC  @CalcAdr$qii 	;вычислить начальный адрес на экране
        PUBLIC  @SPutImg16$qv	;без коорд вывод блока 16*16 на экран
        PUBLIC  @SPutMas16$qv	;без коорд вывод блока 16*16 с прозрачным цветом
				;парам в curadr & curadr1
        PUBLIC  @SPutMas20$qv	;без коорд вывод блока 20*16 с прозрачным цветом
				;парам в curadr & curadr1
        PUBLIC  @FastPutMas$qv	;без коорд вывод блока с прозрачным цветом !!!
				;парам в curadr & curadr1
	PUBLIC	@gettic$qv	;взять тик

;Переменные
	PUBLIC	_CurrentScreen
	PUBLIC	_CurrentColor
	PUBLIC	_CurrentAdr
	PUBLIC	_CurrentAdr1
	PUBLIC	_MaxX
	PUBLIC	_MaxY
	PUBLIC	_MinX
	PUBLIC	_MinY

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE
Copyright       db " FaMe Graph v 2.4 (c)1996 FaMe by BMV 05/08/96 10:20 "

_CurrentScreen  dd	0A0000000h
_VScreen	dd	0A0000000h ;всегда визуальный экран
_CurrentAdr	dd	0A0000000h
_CurrentAdr1	dd	(?)
_CurrentColor   db      (?)
_MaxX		dw      319
_MaxY           dw      199
_MinX           dw      0
_MinY           dw      0

;_tic		dd	(?)	;текущий тик
	ENDS

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE
;	void Vga256();	/* video mode 320x200x256 */

	PROC	@Vga256$qv	FAR

	;push	bp
	;mov	bp,sp
	push	ds

	mov	ax,13h
	int	10h

	pop	ds
	;pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

	PROC	@SetScreen$qnv	FAR	;уст адрес вывода графики и тп
	ARG	buf:DWORD

	push	bp
	mov	bp,sp

	mov	ax,[WORD HIGH buf]	;seg
	mov	[WORD HIGH _CurrentScreen],ax
	mov	ax,[WORD LOW buf]	;offset
	mov	[WORD LOW _CurrentScreen],ax

	pop	bp

	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

	PROC	@NormalScreen$qv FAR	;уст адрес вывода графики в A000

	push	bp
	mov	bp,sp

	mov	[WORD HIGH _CurrentScreen],0A000h
	mov	[WORD LOW _CurrentScreen],0

	pop	bp

	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

        PROC    @WVR$qv	FAR     ;ожидание взм. отрисовки луча

        push    dx

        mov     dx,3DAh
@@WVR1: ;ждем конца текушего вертикального возврата луча
        in      al,dx                   ; port 3DAh, CGA/EGA vid status
        test    al,8
        jnz      @@WVR1                  ; Jump if not zero

        ;pop     dx	;!!!!!!!!!!!!!!!!
	;ret             ;!!!!!!!!!!!!!!!!
@@WVR2: ;ждем начала вертикального возврата луча
        in      al,dx                   ; port 3DAh, CGA/EGA vid status
        test    al,8
        jz     @@WVR2                  ; Jump if not zero

        pop     dx

	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

        PROC    @ScreenCopy$qnvnv      FAR    ;копирование buf1 -> buf
;копирование экрана из buf1 в buf

        ARG     buf:DWORD,buf1:DWORD

	push	bp
	mov	bp,sp
	push	ds

; необходимо переслать из DS:SI -> ES:DI
	mov	ax,[WORD HIGH buf1]	;seg
	mov	ds,ax
	mov	si,[WORD LOW buf1]	;offset

	mov	ax,[WORD HIGH buf]	;seg
	mov	es,ax
	mov	di,[WORD LOW buf]	;offset

        cld
	mov	cx,(64000/2)
	rep	movsw

	pop	ds
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

        PROC    @CopyToScreen$qnv   FAR     ;копирование buf -> экран
;копирование на экран из buf

        ARG     buf:DWORD

	push	bp
	mov	bp,sp
	push	ds

; необходимо переслать из DS:SI -> ES:DI
        mov     ax,[WORD HIGH buf]     ;seg
	mov	ds,ax
        mov     si,[WORD LOW buf]      ;offset

        mov     ax,0A000h
	mov	es,ax
        mov     di,0

        cld

;-------------------------------ожидание для вывода
        mov     dx,3DAh
@@WVR1:
        in	al,dx                   ; port 3DAh, CGA/EGA vid status
        test	al,8
        jnz	@@WVR1			; Jump if not zero
@@WVR2:
        in      al,dx			; port 3DAh, CGA/EGA vid status
        test    al,8
        jz     @@WVR2			; Jump if zero
;-----------------------------------------------------
	mov	cx,(64000/2)
	rep	movsw

	pop	ds
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	Cls(int n)	;очистка экрана

	PROC	@Cls$qzc	FAR
	ARG	n:BYTE

	push	bp
	mov	bp,sp

; необходимо очистить ES:DI
	mov	ax,[WORD HIGH _CurrentScreen]	;seg
	mov	es,ax
	mov	di,[WORD LOW _CurrentScreen]	;offset

	cld
	mov	al,[n]
	mov	ah,[n]
	mov	cx,(64000/2)
	rep	stosw

	pop	bp
	ret
	ENDP

	ENDS

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	SetColor(int n)	;установка текущего цвета

	PROC	@SetColor$qzc	FAR
	ARG	n:WORD

	push	bp
	mov	bp,sp

	mov	al,[BYTE LOW n]
	mov	[_CurrentColor],al

	pop	bp
	ret
	ENDP

	ENDS

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	Clip(int MinX,int MinY,int MaxX,int MaxY) ;установка окна обрезки

	PROC	@Clip$qiiii	FAR
	ARG	MinX:WORD, MinY:WORD, MaxX:WORD, MaxY:WORD

	push	bp
	mov	bp,sp

	mov	ax,[MinX]
	mov	[_MinX],ax

	mov	ax,[MinY]
	mov	[_MinY],ax

	mov	ax,[MaxX]
	mov	[_MaxX],ax

	mov	ax,[MaxY]
	mov	[_MaxY],ax

	pop	bp
	ret
	ENDP

	ENDS

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutPixel(int X,int Y) ;вывод символа текущим цветом

	PROC	@PutPixel$qii	FAR
	ARG	x:WORD, y:WORD

	push	bp
	mov	bp,sp
	push	ax
	push	bx

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel

	mov     al,[_CurrentColor]
	mov	[es:bx],al

	pop	bx
	pop	ax
	pop	bp

	ret
	ENDP

	ENDS

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	int GetPixel(int X,int Y) ;вывод символа текущим цветом

	PROC	@GetPixel$qii	FAR
	ARG	x:WORD, y:WORD

	push	bp
	mov	bp,sp

	mov	ax,[y]
	mov	dx,140h
	imul	dx			; dx:ax = reg * ax
	add	ax,[x]
	mov     es,[WORD HIGH _CurrentScreen]
	add     ax,[WORD LOW _CurrentScreen]
	mov	bx,ax
	mov	al,[es:bx]
	mov	ah,0
	jmp	short $+2		; delay for I/O

	pop	bp
	ret
	ENDP

	ENDS

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	Bar(int x,int y,int xl,int yl) ; Закрашенный прямоугольник
	PROC	@Bar$qiiii	FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD

	push	bp		; preserve caller registers
	mov	bp,sp
	push	ax
	push	bx
	push	cx

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
        mov     di,bx           ; ES:di ->buf

	mov	al,[_CurrentColor]	;цвет
	mov	cx,[yl]
	mov	bx,[xl]

	;cld
@@local1:
	push	cx
	mov	cx,bx
	rep	stosb		;закрасим строчку...
	sub	di,bx
	add	di,320
	pop	cx
	loop	@@local1

	pop	cx
	pop	bx
	pop	ax
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutSImg(x,y,xl,yl,xr,yr,xrl,yrl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutSImg$qiiiiiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, xr:WORD, yr:WORD, xrl:WORD, yrl:WORD, buf:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]
	mov	es,[WORD HIGH _CurrentScreen]
        mov     di,bx           ; ES:di -> адрес первого пиксела для вывода

	mov	ax,[xl]		;вычисляем смещение прямоугольной
	mov	bx,[yr]		;области в спр-те
	mul	bx
	add	ax,[xr]

	add	ax,[WORD LOW buf]
	mov	si,ax
	;проверка на переполнение?!
        mov     ax,[WORD HIGH buf]
	mov	ds,ax

        ;mov     si,[WORD LOW buf]

	mov	cx,[yrl]	;высота образа
@@local2:
	push	cx

	mov	cx,[xrl]	;ширина образа

	rep	movsb		;вывод строчки на экран

	sub	di,[xrl]	;переход на след.строку
	add	di,320		;экрана

	sub	si,[xrl]	;переход на след.строку
	add	si,[xl]         ;внутри большого спр-та

	pop	cx
	loop	@@local2

	pop	ds
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutSMas(x,y,xl,yl,xr,yr,xrl,yrl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutSMas$qiiiiiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, xr:WORD, yr:WORD, xrl:WORD, yrl:WORD, buf:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]
	mov	es,[WORD HIGH _CurrentScreen]
        mov     di,bx           ; ES:di -> адрес первого пиксела для вывода
	DEC	di

	mov	ax,[xl]		;вычисляем смещение прямоугольной
	mov	bx,[yr]		;области в спр-те
	mul	bx
	add	ax,[xr]

	add	ax,[WORD LOW buf]
	mov	si,ax
	;проверка на переполнение?!
        mov     ax,[WORD HIGH buf]
	mov	ds,ax

        ;mov     si,[WORD LOW buf]

	mov	cx,[yrl]	;уст кол-во повторов по Y

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,[xrl]	;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4	;то не выводим

	mov	[es:di],al
@@local4:
	loop	@@local6	;конец цикла по Х

	sub	di,[xrl]	;переход на след.строку
	add	di,320		;экрана

	sub	si,[xrl]	;переход на след.строку
	add	si,[xl]         ;внутри большого спр-та

	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3

	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutSMasr(x,y,xl,yl,xr,yr,xrl,yrl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutSMasr$qiiiiiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, xr:WORD, yr:WORD, xrl:WORD, yrl:WORD, buf:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x
	add	bx,[xrl]

	add	bx,[WORD LOW _CurrentScreen]
	mov	es,[WORD HIGH _CurrentScreen]
        mov     di,bx           ; ES:di -> адрес первого пиксела для вывода
	inc	di

	mov	ax,[xl]		;вычисляем смещение прямоугольной
	mov	bx,[yr]		;области в спр-те
	mul	bx
	add	ax,[xr]

	add	ax,[WORD LOW buf]
	mov	si,ax
	;проверка на переполнение?!
        mov     ax,[WORD HIGH buf]
	mov	ds,ax

        ;mov     si,[WORD LOW buf]

	mov	cx,[yrl]	;уст кол-во повторов по Y

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,[xrl]	;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	dec	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4	;то не выводим

	mov	[es:di],al
@@local4:
	loop	@@local6	;конец цикла по Х

	;sub	di,[xrl]	;переход на след.строку
	add	di,320		;экрана
	add	di,[xrl]

	sub	si,[xrl]	;переход на след.строку
	add	si,[xl]         ;внутри большого спр-та

	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3

	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutSBlink(x,y,xl,yl,xr,yr,xrl,yrl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutSBlink$qiiiiiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, xr:WORD, yr:WORD, xrl:WORD, yrl:WORD, buf:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]
	mov	es,[WORD HIGH _CurrentScreen]
        mov     di,bx           ; ES:di -> адрес первого пиксела для вывода
	dec	di

	mov	ax,[xl]		;вычисляем смещение прямоугольной
	mov	bx,[yr]		;области в спр-те
	mul	bx
	add	ax,[xr]

	add	ax,[WORD LOW buf]
	mov	si,ax
	;проверка на переполнение?!
        mov     ax,[WORD HIGH buf]
	mov	ds,ax

        ;mov     si,[WORD LOW buf]

	mov	cx,[yrl]	;уст кол-во повторов по Y
	mov	ah,[_CurrentColor]	;запомним тек цвет

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,[xrl]	;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4	;то не выводим

	mov	[es:di],ah
@@local4:
	loop	@@local6	;конец цикла по Х

	sub	di,[xrl]	;переход на след.строку
	add	di,320		;экрана

	sub	si,[xrl]	;переход на след.строку
	add	si,[xl]         ;внутри большого спр-та

	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3

	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutSBlinkr(x,y,xl,yl,xr,yr,xrl,yrl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutSBlinkr$qiiiiiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, xr:WORD, yr:WORD, xrl:WORD, yrl:WORD, buf:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x
	add	bx,[xrl]

	add	bx,[WORD LOW _CurrentScreen]
	mov	es,[WORD HIGH _CurrentScreen]
        mov     di,bx           ; ES:di -> адрес первого пиксела для вывода
	inc	di

	mov	ax,[xl]		;вычисляем смещение прямоугольной
	mov	bx,[yr]		;области в спр-те
	mul	bx
	add	ax,[xr]

	add	ax,[WORD LOW buf]
	mov	si,ax
	;проверка на переполнение?!
        mov     ax,[WORD HIGH buf]
	mov	ds,ax

        ;mov     si,[WORD LOW buf]

	mov	cx,[yrl]	;уст кол-во повторов по Y
	mov	ah,[_CurrentColor]	;запомним тек цвет

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,[xrl]	;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	dec	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4	;то не выводим

	mov	[es:di],ah
@@local4:
	loop	@@local6	;конец цикла по Х

	;sub	di,[xrl]	;переход на след.строку
	add	di,320		;экрана
	add	di,[xrl]

	sub	si,[xrl]	;переход на след.строку
	add	si,[xl]         ;внутри большого спр-та

	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3

	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ


SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutImg(x,y,xl,yl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutImg$qiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, buf:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
        mov     di,bx           ; ES:di ->buf

        mov     ax,[WORD HIGH buf]
	mov	ds,ax
        mov     si,[WORD LOW buf]

	mov	cx,[yl]
@@local2:
	push	cx

	mov	cx,[xl]

	rep	movsb

	sub	di,[xl]
	add	di,320
	pop	cx
	loop	@@local2

	pop	ds
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ


SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutImg16(x,y,buf)
;	int x,y;
;	long;buf;

	PROC	@PutImg16$qiinzc FAR
	ARG	x:WORD, y:WORD, buf:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
        mov     di,bx	; ES:di ->buf

        mov     ax,[WORD HIGH buf]
	mov	ds,ax
        mov     si,[WORD LOW buf]


	mov	cx,16
@@local2:
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	add	di,320-16
	loop	@@local2

	pop	ds
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	GetImg(x,y,xl,yl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@GetImg$qiiiinzc	FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, buf:DWORD

	push	bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX
        mov     ds,[WORD HIGH _CurrentScreen]   ; DS:BX
        mov     si,bx           ; dS:si ->экран

        mov     ax,[WORD HIGH buf]
        mov     es,ax
        mov     di,[WORD LOW buf]

; необходимо переслать из DS:SI -> ES:DI

	mov	cx,[yl]
@@local0:
	push	cx

	mov	cx,[xl]

	rep	movsb

	sub	si,[xl]
	add	si,320
	pop	cx
	loop	@@local0

	pop	ds
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutMas(x,y,xl,yl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutMas$qiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, buf:DWORD

	push	bp		; preserve caller registers
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
        mov     di,bx           ; ES:di ->buf
	dec	di

        mov     ax,[WORD HIGH buf]
	mov	ds,ax
        mov     si,[WORD LOW buf]

	mov	cx,[yl]		;уст кол-во повторов по Y

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,[xl]		;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4		;то не выводим

	mov	[es:di],al
@@local4:
	loop	@@local6		;конец цикла по Х
	sub	di,[xl]		;вычтем ширину образа
	add	di,320		;прибавим кол-во точек в строке
	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3
	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutBlink(x,y,xl,yl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutBlink$qiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, buf:DWORD

	push	bp		; preserve caller registers
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
        mov     di,bx           ; ES:di ->buf
	dec	di

        mov     ax,[WORD HIGH buf]
	mov	ds,ax
        mov     si,[WORD LOW buf]

	mov	cx,[yl]		;уст кол-во повторов по Y
	mov	ah,[_CurrentColor]	;запомним тек цвет

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,[xl]		;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4	;то не выводим
	mov	[es:di],ah
@@local4:
	loop	@@local6		;конец цикла по Х
	sub	di,[xl]		;вычтем ширину образа
	add	di,320		;прибавим кол-во точек в строке
	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3
	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutMasr(x,y,xl,yl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutMasr$qiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, buf:DWORD

	push	bp		; preserve caller registers
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x
	add	bx,[xl]		; + ширина

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
        mov     di,bx           ; ES:di ->buf
	inc	di

        mov     ax,[WORD HIGH buf]
	mov	ds,ax
        mov     si,[WORD LOW buf]

	mov	cx,[yl]		;уст кол-во повторов по Y

	;cld			;это слева на право

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,[xl]		;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	dec	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4	;то не выводим

	mov	[es:di],al
@@local4:
	loop	@@local6		;конец цикла по Х

	;sub	di,[xl]		;вычтем ширину образа
	add	di,320		;прибавим кол-во точек в строке
	add	di,[xl]

	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3
	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutBlinkr(x,y,xl,yl,buf)
;	int x,y,xl,yl;
;	long;buf;

	PROC	@PutBlinkr$qiiiinzc FAR
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, buf:DWORD

	push	bp		; preserve caller registers
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x
	add	bx,[xl]		; + ширина

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
        mov     di,bx           ; ES:di ->buf
	inc	di

        mov     ax,[WORD HIGH buf]
	mov	ds,ax
        mov     si,[WORD LOW buf]

	mov	cx,[yl]		;уст кол-во повторов по Y
	mov	ah,[_CurrentColor]	;запомним тек цвет
	;cld			;это слева на право

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,[xl]		;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	dec	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4		;то не выводим

	mov	[es:di],ah
@@local4:
	loop	@@local6		;конец цикла по Х

	;sub	di,[xl]		;вычтем ширину образа
	add	di,320		;прибавим кол-во точек в строке
	add	di,[xl]

	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3
	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutMas16(x,y,buf)
;	int x,y;
;	long;buf;

	PROC	@PutMas16$qiinzc FAR
	ARG	x:WORD, y:WORD, buf:DWORD

	push	bp		; preserve caller registers
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
        mov     di,bx           ; ES:di ->buf
	dec	di

        mov     ax,[WORD HIGH buf]
	mov	ds,ax
        mov     si,[WORD LOW buf]

	mov	cx,16		;уст кол-во повторов по Y

@@local3:
	mov	bx,cx		;сохраним внешн счетчик цикла Y

	mov	cx,16		;установим кол-во повторов по коорд Х
@@local6:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@local4		;то не выводим

	mov	[es:di],al	;иначе выводим
@@local4:
	loop	@@local6		;конец цикла по Х
	;sub	di,[xl]		;вычтем ширину образа
	add	di,320-16		;прибавим кол-во точек в строке
	mov	cx,bx		; восст внешн счетчик цикла Y
	loop	@@local3
	pop	ds		;восст регистр сегмента данных
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ


SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	PutRGB(n,r,g,b)

	PROC	@PutRGB$qzczczczc FAR
	ARG	n:BYTE,r:BYTE,g:BYTE,b:BYTE

	push	bp		; preserve caller registers
	mov	bp,sp

	mov	dx,3c8h
	mov	al,[n]
	out	dx,al

	mov	cx,2
@@loop1:
	loop @@loop1

	inc dx
	mov	al,[r]
	out	dx,al
	mov	al,[g]
	out	dx,al
	mov	al,[b]
	out	dx,al

	mov	sp,bp
	pop	bp
	ret
	ENDP


	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	GetPalette(buf)
;	long;buf;

	PROC	@GetPalette$qnzc FAR
	ARG	buf:DWORD

	push	bp		; preserve caller registers
	mov	bp,sp

	mov	ax,[WORD HIGH buf]	;seg
	mov	es,ax
	mov	dx,[WORD LOW buf]	;offset
	mov	bx,0			;установ N первого регистра палитры
	mov	cx,256			;кол-во регистров палитры

	mov	ax,1017h		;функция считывания палитры
	int	10h

	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

FontOffset     dw      0               ; byte offset of (0,0)
FontSeg  dw      0C000h

;	void DisplayChar(c,x,y,fgd,bkgd);
;	int c;			/* character code */
;	int x,y;		/* upper left pixel */
;	char fgd,bkgd;		/* foreground and background*/

	PROC	@DisplayChar$qiiizczc FAR
	ARG	c:BYTE, x:WORD, y:WORD, fgd:BYTE, bkgd:BYTE

	push	bp		; preserve caller registers
	mov	bp,sp
	push	si
	push	di
	push	ds

; calculate first pixel address

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH _CurrentScreen]	; ES:BX := byte address of pixel
		mov     di,bx           ; ES:di ->buf

		mov     cx,8	; 8 - высота символа

		mov     ax,[WORD FontSeg]      ;seg
		mov     ds,ax
		mov     si,[WORD FontOffset]       ;offset

	mov	al,[c]		; AL := character code
	xor	ah,ah
	mul	cl		; AX := offset into char def table
				;  (POINTS * char code)
	add	si,ax		; SI := addr of char def

; store the character in the video buffer

	mov	bl,[fgd]	; BL := foreground pixel value
	mov	bh,[bkgd]	; BH := background pixel value

	mov	cx,8
@@L102:	push	cx		; preserve CX across loop
	mov	cx,8		; CX := character width in pixels
	lodsb
	mov	ah,al		; AH := bit pattern for next pixel row

@@L112:	mov	al,bl		; AL := foreground pixel value
	shl	ah,1		; carry flag := high-order bit
	jc	@@L122		; jump if bit pattern specifies a
				;  foreground pixel (bit = 1)

	mov	al,bh		; AL := background pixel value
	cmp	bh,0h		;а если пустой цвет?
	jnz	@@L999
	mov	al,[es:di]	; AL := background pixel value
@@L999:

@@L122:	stosb			; update one pixel in the buffer
	loop	@@L112

	add	di,320-8 	; increment buffer address to next
				;  row of pixels
	pop	cx
	loop	@@L102		; loop down character

	pop	ds		; restore registers and return
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;       PutFont(buf)
;	long;buf;

        PROC    @PutFont$qnzc FAR
	ARG	buf:DWORD

	push	bp		; preserve caller registers
	mov	bp,sp

	mov	ax,[WORD HIGH buf]	;seg
        mov     [FontSeg],ax
	mov	dx,[WORD LOW buf]	;offset
        mov     [FontOffset],dx
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	int GetCharWidth(unsigned char c) ;ширина символа

	PROC	@GetCharWidth$qi	FAR
	ARG	c:WORD

	push	bp
	mov	bp,sp

        mov     es,[FontSeg]
        mov     di,[FontOffset]
	add	di,2048
	add	di,[c]

	mov	al,[es:di]
	mov	ah,0
	pop	bp
	ret
	ENDP

	ENDS

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	CopyBlock(x,y,xl,yl,buf,x1,y1,buf1)
;	int x,y,xl,yl;
;	long;buf;

	;PROC	@CopyBlock$qiiiinviinv FAR
	PROC	_CopyBlock
	ARG	x:WORD, y:WORD, xl:WORD, yl:WORD, buf:DWORD, x1:WORD, y1:WORD, buf1:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW buf]	; BX := byte offset in video buffer
	mov	es,[WORD HIGH buf]	; ES:BX := byte address of pixel
        mov     di,bx           ; ES:di ->buf

;--------------------
	mov	ax,[y1]		; AX := y1
	mov	bx,[x1]		; BX := x1

	xchg	ah,al		; AX := 256*y1
	add	bx,ax		; BX := 256*y1 + x1
	shr	ax,1
	shr	ax,1		; AX := 64*y1
	add	bx,ax		; BX := 320*y1 + x1

	add	bx,[WORD LOW buf1]	; BX := byte offset in video buffer
	mov	ds,[WORD HIGH buf1]	; ES:BX := byte address of pixel
        mov     si,bx           ; DS:si ->buf1

	mov	cx,[yl]
@@local2:
	push	cx

	mov	cx,[xl]

	rep	movsb

	sub	di,[xl]
	add	di,320

	sub	si,[xl]
	add	si,320

	pop	cx
	loop	@@local2

	pop	ds
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	CopyBlock0(buf)
;	long;buf;

	PROC	@CopyBlock0$qnv FAR
	ARG	buf:DWORD

        push    bp
	mov	bp,sp
	push	ds

	mov	di,320*16+16
	mov	es,[WORD HIGH _VScreen]
	;mov	es,0A000h	; ES:BX := byte address of pixel
        ;mov     di,bx           ; ES:di ->buf

;--------------------
	mov	bx,320*16+16
	add	bx,[WORD LOW buf]	; BX := byte offset in video buffer
	mov	ds,[WORD HIGH buf]	; ES:BX := byte address of pixel
        mov     si,bx           ; DS:si ->buf1


	mov	cx,16*10

;-------------------------------ожидание для вывода
        mov     dx,3DAh
@@WVR1:
        in	al,dx                   ; port 3DAh, CGA/EGA vid status
        test	al,8
        jnz	@@WVR1			; Jump if not zero
@@WVR2:
        in      al,dx			; port 3DAh, CGA/EGA vid status
        test    al,8
        jz     @@WVR2			; Jump if zero
;-------------------------------------------

@@local2:
	push	cx

	mov	cx,16*16/2

	rep	movsw

	add	di,320-16*16

	add	si,320-16*16

	pop	cx
	loop	@@local2

	pop	ds
	mov	sp,bp
	pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	CalcAdr(int X,int Y) ;вычисл адреса

	PROC	@CalcAdr$qii	FAR
	ARG	x:WORD, y:WORD

	push	bp
	mov	bp,sp

	mov	ax,[y]		; AX := y
	mov	bx,[x]		; BX := x

	xchg	ah,al		; AX := 256*y
	add	bx,ax		; BX := 256*y + x
	shr	ax,1
	shr	ax,1		; AX := 64*y
	add	bx,ax		; BX := 320*y + x

	add	bx,[WORD LOW _CurrentScreen]
	mov	[WORD LOW _CurrentAdr],bx
	mov	ax,[WORD HIGH _CurrentScreen]
	mov	[WORD HIGH _CurrentAdr],ax

	pop	bp

	ret
	ENDP

	ENDS

;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ
SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	SPutImg16();	вывод по адресу

	PROC	@SPutImg16$qv FAR
	;push	bp
	;mov	bp,sp
	mov	bx,ds
	;push	ds

	mov	es,[WORD HIGH _CurrentAdr]
	mov	di,[WORD LOW _CurrentAdr]

	mov	ds,[WORD HIGH _CurrentAdr1]
        mov     si,[WORD LOW _CurrentAdr1]

	mov	cx,16
@@local2:
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	movsw
	add	di,320-16
	loop	@@local2

	mov	ds,bx
	;pop	ds
	;pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	SPutMas16()

	PROC	@SPutMas16$qv FAR
	;push	bp
	;mov	bp,sp
	mov	bx,ds

	mov	es,[WORD HIGH _CurrentAdr]
	mov	di,[WORD LOW _CurrentAdr]
	dec	di

	mov	ds,[WORD HIGH _CurrentAdr1]
        mov     si,[WORD LOW _CurrentAdr1]

;скорость пустого вывода ~10896t
;скорость самого полного вывода ~10256

	mov	cx,16		;уст кол-во повторов по Y
@@begstr:
	lodsb			;считаем из буфера точку	;12t
	inc	di						;2t
	and	al,al		;если равна нулю (прозрачна)	;3t
	jz	@@put01		;то не выводим			;16t | 4t
	mov	[es:di],al	;иначе выводим			;10t

@@put01:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put02		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put02:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put03		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put03:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put04		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put04:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put05		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put05:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put06		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put06:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put07		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put07:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put08		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put08:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put09		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put09:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put10		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put10:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put11		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put11:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put12		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put12:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put13		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put13:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put14		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put14:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put15		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put15:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put16		;то не выводим
	mov	[es:di],al	;иначе выводим
@@put16:
	add	di,320-16	;прибавим кол-во точек в строке	;4t
	loop	@@begstr					;17t

	;pop	ds
	mov	ds,bx
	;pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	SPutMas20()

	PROC	@SPutMas20$qv FAR
	;push	bp
	;mov	bp,sp
	mov	bx,ds

	mov	es,[WORD HIGH _CurrentAdr]
	mov	di,[WORD LOW _CurrentAdr]
	dec	di

	mov	ds,[WORD HIGH _CurrentAdr1]
        mov     si,[WORD LOW _CurrentAdr1]

;скорость пустого вывода ~10896t
;скорость самого полного вывода ~10256

	mov	cx,16		;уст кол-во повторов по Y
@@begstr:
	lodsb			;считаем из буфера точку	;12t
	inc	di						;2t
	and	al,al		;если равна нулю (прозрачна)	;3t
	jz	@@put01		;то не выводим			;16t | 4t
	mov	[es:di],al	;иначе выводим			;10t

@@put01:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put02		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put02:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put03		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put03:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put04		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put04:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put05		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put05:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put06		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put06:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put07		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put07:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put08		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put08:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put09		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put09:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put10		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put10:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put11		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put11:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put12		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put12:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put13		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put13:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put14		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put14:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put15		;то не выводим
	mov	[es:di],al	;иначе выводим

@@put15:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put16		;то не выводим
	mov	[es:di],al	;иначе выводим
@@put16:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put17		;то не выводим
	mov	[es:di],al	;иначе выводим
@@put17:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put18		;то не выводим
	mov	[es:di],al	;иначе выводим
@@put18:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put19		;то не выводим
	mov	[es:di],al	;иначе выводим
@@put19:
	lodsb			;считаем из буфера точку
	inc	di
	and	al,al		;если равна нулю (прозрачна)
	jz	@@put20		;то не выводим
	mov	[es:di],al	;иначе выводим
@@put20:
	add	di,320-20	;прибавим кол-во точек в строке	;4t
	loop	@@begstr					;17t

	;pop	ds
	mov	ds,bx
	;pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

;	FastPutMas()

	PROC	@FastPutMas$qv FAR
	;push	bp
	;mov	bp,sp
	mov	bx,ds

	mov	ds,[WORD HIGH _CurrentAdr1]	;массив с данными
        mov     si,[WORD LOW _CurrentAdr1]

	mov	es,[WORD HIGH _CurrentAdr]	;экран
	mov	di,[WORD LOW _CurrentAdr]

	xor	ch,ch
;скорость пустого вывода ~105t
;скорость самого полного вывода ~5280

@@begstr:
	lodsb		;12t
	cmp al,0
	je @@skipadr	;=0 - перескок адреса ;16t | 4t

	cmp al,2
	lodsb
	mov cl,al
	je @@putword	;=2 - строка из word  ;16t | 4t
	jg @@putdword	;=4 - строка из dword ;16t | 4t
@@putbyte:		;=1 - строка из byte
	rep movsb       ;18t ***
	jmp @@begstr    ;15t
@@putdword:
	rep movsd       ;28t ***
	jmp @@begstr    ;15t
@@putword:
	rep movsw       ;26t ***
	jmp @@begstr    ;15t
@@skipadr:
	lodsw           ;16t
;если прибавка адреса - ноль -выход
	and	ax,ax
	jz	@@end
	add	di,ax	;прибавим кол-во пустых проскоков ;3t
	jmp	@@begstr ;15t
@@end:
	;pop	ds
	mov	ds,bx
	;pop	bp
	ret

	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

SEGMENT	CODE	WORD
	ASSUME	cs:CODE

	;gettic(void)

	PROC	@gettic$qv FAR
        ;push    bp
	mov	ah,0
	int	1ah
	mov	ax,dx	;итог в DX AX
	mov	dx,cx	;итог в DX AX
	jmp	short $+2		; delay for I/O
	;pop	bp
	ret
	ENDP

	ENDS
;ЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁЁ

	END