; PlankaMath 8086 helper.
; AX + BX -> AX.

.MODEL SMALL
.CODE

PUBLIC _pm_i16_add

_pm_i16_add PROC
    ADD AX, BX
    RET
_pm_i16_add ENDP

END
