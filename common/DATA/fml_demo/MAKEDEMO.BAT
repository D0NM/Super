@echo off
@echo Создание ДЕМО-версии

rem graph.fml
fml graph -WEBBER.BIN -STONEMEN.BIN -BAT.BIN -SLIME.BIN -PIG.BIN -GRSLIM.BIN
fml graph -ELSNAKE.BIN -SLONPOT.BIN -GLAZ.BIN -HAND.BIN -MEAT.BIN -SMEN.BIN -worm_c.bin
fml graph -SHARK.BIN -SEACAT.BIN -SNOWMEN.BIN -GHOST.BIN -RAT.BIN -RAMA03.BIN -RAMA02.BIN -RAMA01.BIN
fml graph -RAMA05.BIN -RAMA06.BIN -RAMA07.BIN -RAMA04.BIN -pooh.cfg
fml graph -boss2.BIN -boss3.BIN -boss4.BIN -boss5.BIN -boss6.BIN -boss7.BIN 
fml graph -tukki.bin -kolobok.bin -camera1.bin -camera2.bin -robot1.bin

rem levels.fml
fml levels -02BCKG.BIN -02FON.BIN -02level.bin -02blevel.bin -02models.bin
fml levels -02monstr.bin -02def.bin -02def.bin -02level.bin -02blevel.bin
fml levels -03BCKG.BIN -03FON.BIN -03level.bin -03blevel.bin -03models.bin
fml levels -03monstr.bin -03def.bin -04def.bin -04level.bin -04blevel.bin
fml levels -04monstr.bin -04models.bin -05FON.BIN -05def.bin -05level.bin
fml levels -05blevel.bin -05monstr.bin -05models.bin -05BCKG.BIN -04BCKG.BIN
fml levels -07def.bin -07level.bin -07blevel.bin -07monstr.bin -07models.bin
fml levels -07FON.BIN -07BCKG.BIN -06def.bin -06level.bin -06blevel.bin
fml levels -06monstr.bin -06models.bin -06FON.BIN -06BCKG.BIN -03demo.bin -02demo.bin
fml levels -04demo.bin -05demo.bin -06demo.bin -07demo.bin -03MONSTR.DEF -02MONSTR.DEF
fml levels -04MONSTR.DEF -05MONSTR.DEF -07MONSTR.DEF -06MONSTR.DEF -03STRIP.DEF -02STRIP.DEF
fml levels -04STRIP.DEF -05STRIP.DEF -06STRIP.DEF -07STRIP.DEF

fml mult -03level.scr -03FRAME1.BIN -03FRAME3.PIC -04LEVEL.SCR -04FRAME1.BIN -04FRAME2.PIC -04FRAME3.PIC -04FRAME4.PIC
