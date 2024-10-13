uint8_t linux_keycodes[] = {
    0xff, // 00 
    0xff, // 01 
    0xff, // 02 
    0xff, // 03 
    0xff, // 04 
    0xff, // 05 
    0xff, // 06 
    0xff, // 07 
    0xff, // 08 
    0x45, // 09 escape
    0x01, // 0A 1
    0x02, // 0B 2
    0x03, // 0C 3
    0x04, // 0D 4
    0x05, // 0E 5
    0x06, // 0F 6
    0x07, // 10 7
    0x08, // 11 8
    0x09, // 12 9
    0x0a, // 13 0
    0x0b, // 14 -
    0x0c, // 15 =
    0x41, // 16 backspace
    0x42, // 17 tab
    0x10, // 18 q
    0x11, // 19 w
    0x12, // 1A e
    0x13, // 1B r
    0x14, // 1C t
    0x15, // 1D y
    0x16, // 1E u
    0x17, // 1F i
    0x18, // 20 o
    0x19, // 21 p
    0x1a, // 22 [
    0x1b, // 23 ]
    0x44, // 24 return
    0x63, // 25 ctrl
    0x20, // 26 a
    0x21, // 27 s
    0x22, // 28 d
    0x23, // 29 f
    0x24, // 2A g
    0x25, // 2B h
    0x26, // 2C j
    0x27, // 2D k
    0x28, // 2E l
    0x29, // 2F ;
    0x2a, // 30 '
    0x00, // 31 `
    0x60, // 32 lshift
    0x2b, // 33 # / international 1
    0x31, // 34 z
    0x32, // 35 x
    0x33, // 36 c
    0x34, // 37 v
    0x35, // 38 b
    0x36, // 39 n
    0x37, // 3A m
    0x38, // 3B ,
    0x39, // 3C .
    0x3a, // 3D /
    0x61, // 3E rshift
    0x5d, // 3F numpad asterisk
    0x64, // 40 lalt
    0x40, // 41 space
    0x62, // 42 caps lock
    0x50, // 43 F1
    0x51, // 44 F2
    0x52, // 45 F3
    0x53, // 46 F4
    0x54, // 47 F5
    0x55, // 48 F6
    0x56, // 49 F7
    0x57, // 4A F8
    0x58, // 4B F9
    0x59, // 4C F10
    0x5a, // 4D numlock -> numpad ( 
    0xff, // 4E 
    0x3d, // 4F numpad 7
    0x3e, // 50 numpad 8
    0x3f, // 51 numpad 9
    0x4a, // 52 numpad -
    0x2d, // 53 numpad 4
    0x2e, // 54 numpad 5
    0x2f, // 55 numpad 6
    0x5e, // 56 numpad +
    0x1d, // 57 numpad 1
    0x1e, // 58 numpad 2
    0x1f, // 59 numpad 3
    0x0f, // 5A numpad 0
    0x3c, // 5B numpad .
    0xff, // 5C 
    0xff, // 5D 
    0x30, // 5E | / international 2
    0x5f, // 5F F11 -> help (for keyboards missing insert)
    0x67, // 60 F12 -> right amiga (for keyboard missing right windows and menu)
    0xff, // 61 
    0xff, // 62 
    0xff, // 63 
    0xff, // 64 
    0xff, // 65 
    0xff, // 66 
    0xff, // 67 
    0x43, // 68 numpad enter
    0x63, // 69 right ctrl -> ctrl
    0x5c, // 6A numpad /
    0x6d, // 6B printscreen
    0x65, // 6C altgr -> ralt
    0xff, // 6D 
    0x70, // 6E home
    0x4c, // 6F curs up
    0x48, // 70 page up
    0x4f, // 71 curs left
    0x4e, // 72 curs right
    0x71, // 73 end
    0x4d, // 74 curs down
    0x49, // 75 page down
    0x5f, // 76 insert -> help
    0x46, // 77 delete
    0xff, // 78 
    0xff, // 79 
    0xff, // 7A 
    0xff, // 7B 
    0xff, // 7C 
    0xff, // 7D 
    0xff, // 7E 
    0x6e, // 7F pause / break
    0xff, // 80 
    0xff, // 81 
    0xff, // 82 
    0xff, // 83 
    0xff, // 84 
    0x66, // 85 left windows -> left amiga
    0x67, // 86 right windows -> right amiga
    0x67, // 87 menu key -> right amiga
};

uint8_t mac_keycodes[] = {
    0xff, // 00 
    0x20, // 01 a
    0x21, // 02 s
    0x22, // 03 d
    0x23, // 04 f
    0x25, // 05 h
    0x24, // 06 g
    0x31, // 07 z
    0x32, // 08 x
    0x33, // 09 c
    0x34, // 0A v
    0x00, // 0B ~
    0x35, // 0C b
    0x10, // 0D q
    0x11, // 0E w
    0x12, // 0F e
    0x13, // 10 r
    0x15, // 11 y
    0x14, // 12 t
    0x01, // 13 1
    0x02, // 14 2
    0x03, // 15 3
    0x04, // 16 4
    0x06, // 17 6
    0x05, // 18 5
    0x0c, // 19 =
    0x09, // 1A 9
    0x07, // 1B 7
    0x0b, // 1C -
    0x08, // 1D 8
    0x0a, // 1E 0
    0x1b, // 1F ]
    0x18, // 20 o
    0x16, // 21 u
    0x1a, // 22 [
    0x17, // 23 i
    0x19, // 24 p
    0x44, // 25 return
    0x28, // 26 l
    0x26, // 27 j
    0x2a, // 28 "
    0x27, // 29 k
    0x29, // 2A :
    0x2b, // 2B # / international 1
    0x38, // 2C <
    0x3a, // 2D ?
    0x36, // 2E n
    0x37, // 2F m
    0x39, // 30 >
    0x42, // 31 tab
    0x40, // 32 space
    0x30, // 33 | / international 2
    0x41, // 34 backspace
    0xff, // 35 
    0x45, // 36 escape
    0x67, // 37 right cmd -> right amiga
    0x66, // 38 left cmd -> left amiga
    0x60, // 39 left shift
    0x62, // 3A caps lock
    0x64, // 3B left alt
    0x63, // 3C left ctrl -> ctrl
    0x61, // 3D right shift
    0x65, // 3E right alt
    0x63, // 3F right ctrl -> ctrl
    0xff, // 40 
    0xff, // 41 
    0x3c, // 42 Numpad . 
    0xff, // 43 
    0x5d, // 44 numpad *
    0xff, // 45 
    0x5e, // 46 numpad + 
    0xff, // 47 
    0x5a, // 48 numlock -> numpad ( 
    0xff, // 49 
    0xff, // 4A 
    0xff, // 4B 
    0x5c, // 4C numpad /
    0x43, // 4D numpad enter
    0xff, // 4E 
    0x4a, // 4F numpad - 
    0xff, // 50 
    0xff, // 51 
    0xff, // 52 
    0x0f, // 53 numpad 0 
    0x1d, // 54 numpad 1 
    0x1e, // 55 numpad 2 
    0x1f, // 56 numpad 3
    0x2d, // 57 numpad 4
    0x2e, // 58 numpad 5
    0x2f, // 59 numpad 6
    0x3d, // 5A numpad 7 
    0xff, // 5B 
    0x3e, // 5C numpad 8 
    0x3f, // 5D numpad 9
    0xff, // 5E 
    0xff, // 5F 
    0xff, // 60 
    0x54, // 61 F5
    0x55, // 62 F6
    0x56, // 63 F7
    0x52, // 64 F3
    0x57, // 65 F8
    0x58, // 66 F9
    0xff, // 67
    0x5f, // 68 F11 -> help (for small mac keyboards without insert)
    0xff, // 69 
    0x6d, // 6A print screen
    0xff, // 6B 
    0xff, // 6C 
    0xff, // 6D 
    0x59, // 6E F10
    0x67, // 6F menu key -> right amiga
    0x67, // 70 F12 -> right amiga (for synergy server that does not send right cmd key)
    0xff, // 71 
    0x6e, // 72 pause / break
    0x5f, // 73 insert/help -> help 
    0x70, // 74 home
    0x48, // 75 page up
    0x46, // 76 delete
    0x53, // 77 F4
    0x71, // 78 end
    0x51, // 79 F2 
    0x49, // 7A page down
    0x50, // 7B F1
    0x4f, // 7C curs left
    0x4e, // 7D curs right
    0x4d, // 7E curs down
    0x4c, // 7F curs up 
};

uint8_t windows_keycodes[] = {
    0xff, // 00
    0x45, // 01 esc
    0x01, // 02 1
    0x02, // 03 2
    0x03, // 04 3
    0x04, // 05 4
    0x05, // 06 5
    0x06, // 07 6
    0x07, // 08 7
    0x08, // 09 8
    0x09, // 0A 9
    0x0a, // 0B 0
    0x0b, // 0C -
    0x0c, // 0D =
    0x41, // 0E backspace
    0x42, // 0F tab
    0x10, // 10 q
    0x11, // 11 w
    0x12, // 12 e
    0x13, // 13 r
    0x14, // 14 t
    0x15, // 15 y
    0x16, // 16 u
    0x17, // 17 i
    0x18, // 18 o
    0x19, // 19 p
    0x1a, // 1A {
    0x1b, // 1B }
    0x44, // 1C return
    0x63, // 1D ctrl
    0x20, // 1E a
    0x21, // 1F s
    0x22, // 20 d
    0x23, // 21 f
    0x24, // 22 g
    0x25, // 23 h
    0x26, // 24 j
    0x27, // 25 k
    0x28, // 26 l
    0x29, // 27 :
    0x2a, // 28 "
    0x00, // 29 `
    0x60, // 2A left shift
    0x2b, // 2B # / international 1
    0x31, // 2C z
    0x32, // 2D x
    0x33, // 2E c
    0x34, // 2F v
    0x35, // 30 b
    0x36, // 31 n
    0x37, // 32 m
    0x38, // 33 <
    0x39, // 34 >
    0x3a, // 35 ?
    0x61, // 36 right shift
    0x5d, // 37 numpad *
    0x64, // 38 left alt
    0x40, // 39 space 
    0x62, // 3A caps lock
    0x50, // 3B F1
    0x51, // 3C F2
    0x52, // 3D F3
    0x53, // 3E F4
    0x54, // 3F F5
    0x55, // 40 F6
    0x56, // 41 F7
    0x57, // 42 F8
    0x58, // 43 F9
    0x59, // 44 F10
    0x6e, // 45 pause / break
    0xff, // 46
    0x3d, // 47 numpad 7
    0x3e, // 48 numpad 8
    0x3f, // 49 numpad 9
    0x4a, // 4A numpad -
    0x2d, // 4B numpad 4
    0x2e, // 4C numpad 5
    0x2f, // 4D numpad 6
    0x5e, // 4E numpad +
    0x1d, // 4F numpad 1
    0x1e, // 50 numpad 2
    0x1f, // 51 numpad 3
    0x0f, // 52 numpad 0
    0x3c, // 53 numpad .
    0xff, // 54
    0xff, // 55
    0x30, // 56 # / international 1
    0x5f, // 57 F11 -> help (for keyboards missing insert)
    0x67, // 58 F12 -> right amiga (for keyboards missing right windows and menu)
    0xff, // 59
    0xff, // 5A
    0xff, // 5B
    0xff, // 5C
    0xff, // 5D
    0xff, // 5E
    0xff, // 5F
    0xff, // 60
    0xff, // 61
    0xff, // 62
    0xff, // 63
    0xff, // 64
    0xff, // 65
    0xff, // 66
    0xff, // 67
    0xff, // 68
    0xff, // 69
    0xff, // 6A
    0xff, // 6B
    0xff, // 6C
    0xff, // 6D
    0xff, // 6E
    0xff, // 6F
    0xff, // 70
    0xff, // 71 
    0xff, // 72 
    0xff, // 73 
    0xff, // 74 
    0xff, // 75 
    0xff, // 76
    0xff, // 77
    0xff, // 78
    0xff, // 79
    0xff, // 7A
    0xff, // 7B
    0x4f, // 7C curs left
    0x4e, // 7D curs right
    0x4d, // 7E curs down
    0x4c, // 7F curs up 
    0xff, // 80 
    0xff, // 81 
    0xff, // 82 
    0xff, // 83 
    0xff, // 84 
    0xff, // 85 
    0xff, // 86 
    0xff, // 87 
    0xff, // 88 
    0xff, // 89 
    0xff, // 8A 
    0xff, // 8B 
    0xff, // 8C 
    0xff, // 8D 
    0xff, // 8E 
    0xff, // 8F 
    0xff, // 90 
    0xff, // 91 
    0xff, // 92 
    0xff, // 93 
    0xff, // 94 
    0xff, // 95 
    0xff, // 96 
    0xff, // 97 
    0xff, // 98 
    0xff, // 99 
    0xff, // 9A 
    0xff, // 9B 
    0xff, // 9C 
    0xff, // 9D 
    0xff, // 9E 
    0xff, // 9F 
    0xff, // A0
    0xff, // A1
    0xff, // A2
    0xff, // A3
    0xff, // A4
    0xff, // A5
    0xff, // A6
    0xff, // A7
    0xff, // A8
    0xff, // A9
    0xff, // AA
    0xff, // AB
    0xff, // AC
    0xff, // AD
    0xff, // AE
    0xff, // AF
    0xff, // B0
    0xff, // B1
    0xff, // B2
    0xff, // B3
    0xff, // B4
    0xff, // B5
    0xff, // B6
    0xff, // B7
    0xff, // B8
    0xff, // B9
    0xff, // BA
    0xff, // BB
    0xff, // BC
    0xff, // BD
    0xff, // BE
    0xff, // BF
    0xff, // C0
    0xff, // C1
    0xff, // C2
    0xff, // C3
    0xff, // C4
    0xff, // C5
    0xff, // C6
    0xff, // C7
    0xff, // C8
    0xff, // C9
    0xff, // CA
    0xff, // CB
    0xff, // CC
    0xff, // CD
    0xff, // CE
    0xff, // CF
    0xff, // D0
    0xff, // D1
    0xff, // D2
    0xff, // D3
    0xff, // D4
    0xff, // D5
    0xff, // D6
    0xff, // D7
    0xff, // D8
    0xff, // D9
    0xff, // DA
    0xff, // DB
    0xff, // DC
    0xff, // DD
    0xff, // DE
    0xff, // DF
    0xff, // E0
    0xff, // E1
    0xff, // E2
    0xff, // E3
    0xff, // E4
    0xff, // E5
    0xff, // E6
    0xff, // E7
    0xff, // E8
    0xff, // E9
    0xff, // EA
    0xff, // EB
    0xff, // EC
    0xff, // ED
    0xff, // EE
    0xff, // EF
    0xff, // F0
    0xff, // F1
    0xff, // F2
    0xff, // F3
    0xff, // F4
    0xff, // F5
    0xff, // F6
    0xff, // F7
    0xff, // F8
    0xff, // F9
    0xff, // FA
    0xff, // FB
    0xff, // FC
    0xff, // FD
    0xff, // FE
    0xff, // FF
    0xff, // 100
    0xff, // 101
    0xff, // 102
    0xff, // 103
    0xff, // 104
    0xff, // 105
    0xff, // 106
    0xff, // 107
    0xff, // 108
    0xff, // 109
    0xff, // 10A
    0xff, // 10B
    0xff, // 10C
    0xff, // 10D
    0xff, // 10E
    0xff, // 10F
    0xff, // 110
    0xff, // 111
    0xff, // 112
    0xff, // 113
    0xff, // 114
    0xff, // 115
    0xff, // 116
    0xff, // 117
    0xff, // 118
    0xff, // 119
    0xff, // 11A
    0xff, // 11B
    0x43, // 11C numpad enter
    0x63, // 11D right ctrl -> ctrl
    0xff, // 11E
    0xff, // 11F
    0xff, // 120
    0xff, // 121
    0xff, // 122
    0xff, // 123
    0xff, // 124
    0xff, // 125
    0xff, // 126
    0xff, // 127
    0xff, // 128
    0xff, // 129
    0xff, // 12A
    0xff, // 12B
    0xff, // 12C
    0xff, // 12D
    0xff, // 12E
    0xff, // 12F
    0xff, // 130
    0xff, // 131
    0xff, // 132
    0xff, // 133
    0xff, // 134
    0x5c, // 135 numpad /
    0xff, // 136
    0x6d, // 137 print screen
    0x65, // 138 right alt
    0xff, // 139
    0xff, // 13A
    0xff, // 13B
    0xff, // 13C
    0xff, // 13D
    0xff, // 13E
    0xff, // 13F
    0xff, // 140
    0xff, // 141
    0xff, // 142
    0xff, // 143
    0xff, // 144
    0x5a, // 145 numlock -> numpad (
    0xff, // 146
    0x70, // 147 home
    0x4c, // 148 curs up
    0x48, // 149 page up
    0xff, // 14A
    0x4f, // 14B curs left
    0xff, // 14C
    0x4e, // 14D curs right
    0xff, // 14E
    0x71, // 14F end
    0x4d, // 150 curs down
    0x49, // 151 page down
    0x5f, // 152 insert -> help
    0x46, // 153 delete
    0xff, // 154
    0xff, // 155
    0xff, // 156
    0xff, // 157
    0xff, // 158
    0xff, // 159
    0xff, // 15A
    0x66, // 15B left windows -> left amiga
    0x67, // 15C right windows -> right amiga
    0x67, // 15D windows menu key -> right amiga
    0xff, // 15E
    0xff, // 15F
};
