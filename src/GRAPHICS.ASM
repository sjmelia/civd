; Loader tail, raster, picture codec and mouse helpers: the second F0000
; contribution, linked after CITYMGT in original relocation order.
.8086
EXTRN _F0000:BYTE, _F2B01:BYTE
F0000 SEGMENT BYTE PUBLIC 'CODE'
INCLUDE F0STATE.INC
INCLUDE LOADTAIL.INC
INCLUDE RASTER.INC
INCLUDE PICTURE.INC
INCLUDE MOUSE.INC
F0000 ENDS
END
