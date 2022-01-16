    .globl           _bos_desc, _msos20_desc
    .area	DESC	(CODE)
VENDOR_MS_OS_20_REQUEST = 0xCC

    .even
_bos_desc:
    .db 0x05
    .db 0x0F
_BOS_DESC_SIZE::
    .db <(bos_desc_end - _bos_desc)
    .db >(bos_desc_end - _bos_desc)
    .db 0x01

    .db 0x1C
    .db 0x10
    .db 0x05
    .db 0x00
    .db 0xDF
    .db 0x60
    .db 0xDD
    .db 0xD8
    .db 0x89
    .db 0x45
    .db 0xC7
    .db 0x4C
    .db 0x9C
    .db 0xD2
    .db 0x65
    .db 0x9D
    .db 0x9E
    .db 0x64
    .db 0x8A
    .db 0x9F
    .db 0x00
    .db 0x00
    .db 0x03
    .db 0x06
    .db <(msos20_desc_end - _msos20_desc)
    .db >(msos20_desc_end - _msos20_desc)
_VENDOR_MS_OS_20_REQUEST::
    .db VENDOR_MS_OS_20_REQUEST
    .db 0x00
bos_desc_end:

; -----------------------------------------------------------------------------
; MS OS 2.0 ( Microsoft OS 2.0 Descriptors )
; -----------------------------------------------------------------------------
    .even
_msos20_desc:
	.db	0x0A, 0x00, 0x00, 0x00
	.db	0x00, 0x00, 0x03, 0x06
_MSOS20_DESC_SIZE::
	.db	<(msos20_desc_end - _msos20_desc)
	.db	>(msos20_desc_end - _msos20_desc)
	.db	0x14, 0x00, 0x03, 0x00
	.db	'W, 'I, 'N, 'U, 'S, 'B, 0, 0
	.db	0, 0, 0, 0, 0, 0, 0, 0
msos20_desc_end:
