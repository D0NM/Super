; $Date: 2002/04/01 00:58:21 $  
; $Id: data.s,v 1.11 2002/04/01 00:58:21 ajo Exp $ 

	AREA sound_data, DATA, READONLY
	
	
	EXPORT my_module

my_module
	INCBIN coming.mod

	END
