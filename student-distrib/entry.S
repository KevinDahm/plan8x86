
 .text

#define SAVE_ALL \
  cld; \
  pushl %fs; \
  pushl %es; \
  pushl %ds; \
  pushl %eax; \
  pushl %ebp; \
  pushl %edi; \
  pushl %esi; \
  pushl %edx; \
  pushl %ecx; \
  pushl %ebx; \

#define RESTORE_ALL \
  popl %ebx; \
	popl %ecx; \
	popl %edx; \
	popl %esi; \
	popl %edi; \
	popl %ebp; \
	popl %eax; \
	popl %ds; \
	popl %es; \
	popl %fs; \

common_interrupt:
  SAVE_ALL
	movl %esp, %ecx
	call do_IRQ
	RESTORE_ALL
  addl $4, %esp
	iret


.globl irq0
irq0:
  pushl $0xFFFFFFFF
  jmp common_interrupt

.global irq1
irq1:
  pushl $0xFFFFFFFE
  jmp common_interrupt

.global irq2
irq2:
  pushl $0xFFFFFFFD
  jmp common_interrupt

.global irq3
irq3:
  pushl $0xFFFFFFFC
  jmp common_interrupt

.global irq4
irq4:
  pushl $0xFFFFFFFB
  jmp common_interrupt

.global irq5
irq5:
  pushl $0xFFFFFFFA
  jmp common_interrupt

.global irq6
irq6:
  pushl $0xFFFFFFF9
  jmp common_interrupt

.global irq7
irq7:
  pushl $0xFFFFFFF8
  jmp common_interrupt

.global irq8
irq8:
  pushl $0xFFFFFFF7
  jmp common_interrupt

.global irq9
irq9:
  pushl $0xFFFFFFF6
  jmp common_interrupt

.global irq10
irq10:
  pushl $0xFFFFFFF5
  jmp common_interrupt

.global irq11
irq11:
  pushl $0xFFFFFFF4
  jmp common_interrupt

.global irq12
irq12:
  pushl $0xFFFFFFF3
  jmp common_interrupt

.global irq13
irq13:
  pushl $0xFFFFFFF2
  jmp common_interrupt

.global irq14
irq14:
  pushl $0xFFFFFFF1
  jmp common_interrupt

.global irq15
irq15:
  pushl $0xFFFFFFF0
  jmp common_interrupt

.global irq16
irq16:
  pushl $0xFFFFFFEF
  jmp common_interrupt

.global irq17
irq17:
  pushl $0xFFFFFFEE
  jmp common_interrupt

.global irq18
irq18:
  pushl $0xFFFFFFED
  jmp common_interrupt

.global irq19
irq19:
  pushl $0xFFFFFFEC
  jmp common_interrupt

.global irq20
irq20:
  pushl $0xFFFFFFEB
  jmp common_interrupt

.global irq21
irq21:
  pushl $0xFFFFFFEA
  jmp common_interrupt

.global irq22
irq22:
  pushl $0xFFFFFFE9
  jmp common_interrupt

.global irq23
irq23:
  pushl $0xFFFFFFE8
  jmp common_interrupt

.global irq24
irq24:
  pushl $0xFFFFFFE7
  jmp common_interrupt

.global irq25
irq25:
  pushl $0xFFFFFFE6
  jmp common_interrupt

.global irq26
irq26:
  pushl $0xFFFFFFE5
  jmp common_interrupt

.global irq27
irq27:
  pushl $0xFFFFFFE4
  jmp common_interrupt

.global irq28
irq28:
  pushl $0xFFFFFFE3
  jmp common_interrupt

.global irq29
irq29:
  pushl $0xFFFFFFE2
  jmp common_interrupt

.global irq30
irq30:
  pushl $0xFFFFFFE1
  jmp common_interrupt

.global irq31
irq31:
  pushl $0xFFFFFFE0
  jmp common_interrupt

.global irq32
irq32:
  pushl $0xFFFFFFDF
  jmp common_interrupt

.global irq33
irq33:
  pushl $0xFFFFFFDE
  jmp common_interrupt

.global irq34
irq34:
  pushl $0xFFFFFFDD
  jmp common_interrupt

.global irq35
irq35:
  pushl $0xFFFFFFDC
  jmp common_interrupt

.global irq36
irq36:
  pushl $0xFFFFFFDB
  jmp common_interrupt

.global irq37
irq37:
  pushl $0xFFFFFFDA
  jmp common_interrupt

.global irq38
irq38:
  pushl $0xFFFFFFD9
  jmp common_interrupt

.global irq39
irq39:
  pushl $0xFFFFFFD8
  jmp common_interrupt

.global irq40
irq40:
  pushl $0xFFFFFFD7
  jmp common_interrupt

.global irq41
irq41:
  pushl $0xFFFFFFD6
  jmp common_interrupt

.global irq42
irq42:
  pushl $0xFFFFFFD5
  jmp common_interrupt

.global irq43
irq43:
  pushl $0xFFFFFFD4
  jmp common_interrupt

.global irq44
irq44:
  pushl $0xFFFFFFD3
  jmp common_interrupt

.global irq45
irq45:
  pushl $0xFFFFFFD2
  jmp common_interrupt

.global irq46
irq46:
  pushl $0xFFFFFFD1
  jmp common_interrupt

.global irq47
irq47:
  pushl $0xFFFFFFD0
  jmp common_interrupt

.global irq48
irq48:
  pushl $0xFFFFFFCF
  jmp common_interrupt

.global irq49
irq49:
  pushl $0xFFFFFFCE
  jmp common_interrupt

.global irq50
irq50:
  pushl $0xFFFFFFCD
  jmp common_interrupt

.global irq51
irq51:
  pushl $0xFFFFFFCC
  jmp common_interrupt

.global irq52
irq52:
  pushl $0xFFFFFFCB
  jmp common_interrupt

.global irq53
irq53:
  pushl $0xFFFFFFCA
  jmp common_interrupt

.global irq54
irq54:
  pushl $0xFFFFFFC9
  jmp common_interrupt

.global irq55
irq55:
  pushl $0xFFFFFFC8
  jmp common_interrupt

.global irq56
irq56:
  pushl $0xFFFFFFC7
  jmp common_interrupt

.global irq57
irq57:
  pushl $0xFFFFFFC6
  jmp common_interrupt

.global irq58
irq58:
  pushl $0xFFFFFFC5
  jmp common_interrupt

.global irq59
irq59:
  pushl $0xFFFFFFC4
  jmp common_interrupt

.global irq60
irq60:
  pushl $0xFFFFFFC3
  jmp common_interrupt

.global irq61
irq61:
  pushl $0xFFFFFFC2
  jmp common_interrupt

.global irq62
irq62:
  pushl $0xFFFFFFC1
  jmp common_interrupt

.global irq63
irq63:
  pushl $0xFFFFFFC0
  jmp common_interrupt

.global irq64
irq64:
  pushl $0xFFFFFFBF
  jmp common_interrupt

.global irq65
irq65:
  pushl $0xFFFFFFBE
  jmp common_interrupt

.global irq66
irq66:
  pushl $0xFFFFFFBD
  jmp common_interrupt

.global irq67
irq67:
  pushl $0xFFFFFFBC
  jmp common_interrupt

.global irq68
irq68:
  pushl $0xFFFFFFBB
  jmp common_interrupt

.global irq69
irq69:
  pushl $0xFFFFFFBA
  jmp common_interrupt

.global irq70
irq70:
  pushl $0xFFFFFFB9
  jmp common_interrupt

.global irq71
irq71:
  pushl $0xFFFFFFB8
  jmp common_interrupt

.global irq72
irq72:
  pushl $0xFFFFFFB7
  jmp common_interrupt

.global irq73
irq73:
  pushl $0xFFFFFFB6
  jmp common_interrupt

.global irq74
irq74:
  pushl $0xFFFFFFB5
  jmp common_interrupt

.global irq75
irq75:
  pushl $0xFFFFFFB4
  jmp common_interrupt

.global irq76
irq76:
  pushl $0xFFFFFFB3
  jmp common_interrupt

.global irq77
irq77:
  pushl $0xFFFFFFB2
  jmp common_interrupt

.global irq78
irq78:
  pushl $0xFFFFFFB1
  jmp common_interrupt

.global irq79
irq79:
  pushl $0xFFFFFFB0
  jmp common_interrupt

.global irq80
irq80:
  pushl $0xFFFFFFAF
  jmp common_interrupt

.global irq81
irq81:
  pushl $0xFFFFFFAE
  jmp common_interrupt

.global irq82
irq82:
  pushl $0xFFFFFFAD
  jmp common_interrupt

.global irq83
irq83:
  pushl $0xFFFFFFAC
  jmp common_interrupt

.global irq84
irq84:
  pushl $0xFFFFFFAB
  jmp common_interrupt

.global irq85
irq85:
  pushl $0xFFFFFFAA
  jmp common_interrupt

.global irq86
irq86:
  pushl $0xFFFFFFA9
  jmp common_interrupt

.global irq87
irq87:
  pushl $0xFFFFFFA8
  jmp common_interrupt

.global irq88
irq88:
  pushl $0xFFFFFFA7
  jmp common_interrupt

.global irq89
irq89:
  pushl $0xFFFFFFA6
  jmp common_interrupt

.global irq90
irq90:
  pushl $0xFFFFFFA5
  jmp common_interrupt

.global irq91
irq91:
  pushl $0xFFFFFFA4
  jmp common_interrupt

.global irq92
irq92:
  pushl $0xFFFFFFA3
  jmp common_interrupt

.global irq93
irq93:
  pushl $0xFFFFFFA2
  jmp common_interrupt

.global irq94
irq94:
  pushl $0xFFFFFFA1
  jmp common_interrupt

.global irq95
irq95:
  pushl $0xFFFFFFA0
  jmp common_interrupt

.global irq96
irq96:
  pushl $0xFFFFFF9F
  jmp common_interrupt

.global irq97
irq97:
  pushl $0xFFFFFF9E
  jmp common_interrupt

.global irq98
irq98:
  pushl $0xFFFFFF9D
  jmp common_interrupt

.global irq99
irq99:
  pushl $0xFFFFFF9C
  jmp common_interrupt

.global irq100
irq100:
  pushl $0xFFFFFF9B
  jmp common_interrupt

.global irq101
irq101:
  pushl $0xFFFFFF9A
  jmp common_interrupt

.global irq102
irq102:
  pushl $0xFFFFFF99
  jmp common_interrupt

.global irq103
irq103:
  pushl $0xFFFFFF98
  jmp common_interrupt

.global irq104
irq104:
  pushl $0xFFFFFF97
  jmp common_interrupt

.global irq105
irq105:
  pushl $0xFFFFFF96
  jmp common_interrupt

.global irq106
irq106:
  pushl $0xFFFFFF95
  jmp common_interrupt

.global irq107
irq107:
  pushl $0xFFFFFF94
  jmp common_interrupt

.global irq108
irq108:
  pushl $0xFFFFFF93
  jmp common_interrupt

.global irq109
irq109:
  pushl $0xFFFFFF92
  jmp common_interrupt

.global irq110
irq110:
  pushl $0xFFFFFF91
  jmp common_interrupt

.global irq111
irq111:
  pushl $0xFFFFFF90
  jmp common_interrupt

.global irq112
irq112:
  pushl $0xFFFFFF8F
  jmp common_interrupt

.global irq113
irq113:
  pushl $0xFFFFFF8E
  jmp common_interrupt

.global irq114
irq114:
  pushl $0xFFFFFF8D
  jmp common_interrupt

.global irq115
irq115:
  pushl $0xFFFFFF8C
  jmp common_interrupt

.global irq116
irq116:
  pushl $0xFFFFFF8B
  jmp common_interrupt

.global irq117
irq117:
  pushl $0xFFFFFF8A
  jmp common_interrupt

.global irq118
irq118:
  pushl $0xFFFFFF89
  jmp common_interrupt

.global irq119
irq119:
  pushl $0xFFFFFF88
  jmp common_interrupt

.global irq120
irq120:
  pushl $0xFFFFFF87
  jmp common_interrupt

.global irq121
irq121:
  pushl $0xFFFFFF86
  jmp common_interrupt

.global irq122
irq122:
  pushl $0xFFFFFF85
  jmp common_interrupt

.global irq123
irq123:
  pushl $0xFFFFFF84
  jmp common_interrupt

.global irq124
irq124:
  pushl $0xFFFFFF83
  jmp common_interrupt

.global irq125
irq125:
  pushl $0xFFFFFF82
  jmp common_interrupt

.global irq126
irq126:
  pushl $0xFFFFFF81
  jmp common_interrupt

.global irq127
irq127:
  pushl $0xFFFFFF80
  jmp common_interrupt

.global irq128
irq128:
  pushl $0xFFFFFF7F
  jmp common_interrupt

.global irq129
irq129:
  pushl $0xFFFFFF7E
  jmp common_interrupt

.global irq130
irq130:
  pushl $0xFFFFFF7D
  jmp common_interrupt

.global irq131
irq131:
  pushl $0xFFFFFF7C
  jmp common_interrupt

.global irq132
irq132:
  pushl $0xFFFFFF7B
  jmp common_interrupt

.global irq133
irq133:
  pushl $0xFFFFFF7A
  jmp common_interrupt

.global irq134
irq134:
  pushl $0xFFFFFF79
  jmp common_interrupt

.global irq135
irq135:
  pushl $0xFFFFFF78
  jmp common_interrupt

.global irq136
irq136:
  pushl $0xFFFFFF77
  jmp common_interrupt

.global irq137
irq137:
  pushl $0xFFFFFF76
  jmp common_interrupt

.global irq138
irq138:
  pushl $0xFFFFFF75
  jmp common_interrupt

.global irq139
irq139:
  pushl $0xFFFFFF74
  jmp common_interrupt

.global irq140
irq140:
  pushl $0xFFFFFF73
  jmp common_interrupt

.global irq141
irq141:
  pushl $0xFFFFFF72
  jmp common_interrupt

.global irq142
irq142:
  pushl $0xFFFFFF71
  jmp common_interrupt

.global irq143
irq143:
  pushl $0xFFFFFF70
  jmp common_interrupt

.global irq144
irq144:
  pushl $0xFFFFFF6F
  jmp common_interrupt

.global irq145
irq145:
  pushl $0xFFFFFF6E
  jmp common_interrupt

.global irq146
irq146:
  pushl $0xFFFFFF6D
  jmp common_interrupt

.global irq147
irq147:
  pushl $0xFFFFFF6C
  jmp common_interrupt

.global irq148
irq148:
  pushl $0xFFFFFF6B
  jmp common_interrupt

.global irq149
irq149:
  pushl $0xFFFFFF6A
  jmp common_interrupt

.global irq150
irq150:
  pushl $0xFFFFFF69
  jmp common_interrupt

.global irq151
irq151:
  pushl $0xFFFFFF68
  jmp common_interrupt

.global irq152
irq152:
  pushl $0xFFFFFF67
  jmp common_interrupt

.global irq153
irq153:
  pushl $0xFFFFFF66
  jmp common_interrupt

.global irq154
irq154:
  pushl $0xFFFFFF65
  jmp common_interrupt

.global irq155
irq155:
  pushl $0xFFFFFF64
  jmp common_interrupt

.global irq156
irq156:
  pushl $0xFFFFFF63
  jmp common_interrupt

.global irq157
irq157:
  pushl $0xFFFFFF62
  jmp common_interrupt

.global irq158
irq158:
  pushl $0xFFFFFF61
  jmp common_interrupt

.global irq159
irq159:
  pushl $0xFFFFFF60
  jmp common_interrupt

.global irq160
irq160:
  pushl $0xFFFFFF5F
  jmp common_interrupt

.global irq161
irq161:
  pushl $0xFFFFFF5E
  jmp common_interrupt

.global irq162
irq162:
  pushl $0xFFFFFF5D
  jmp common_interrupt

.global irq163
irq163:
  pushl $0xFFFFFF5C
  jmp common_interrupt

.global irq164
irq164:
  pushl $0xFFFFFF5B
  jmp common_interrupt

.global irq165
irq165:
  pushl $0xFFFFFF5A
  jmp common_interrupt

.global irq166
irq166:
  pushl $0xFFFFFF59
  jmp common_interrupt

.global irq167
irq167:
  pushl $0xFFFFFF58
  jmp common_interrupt

.global irq168
irq168:
  pushl $0xFFFFFF57
  jmp common_interrupt

.global irq169
irq169:
  pushl $0xFFFFFF56
  jmp common_interrupt

.global irq170
irq170:
  pushl $0xFFFFFF55
  jmp common_interrupt

.global irq171
irq171:
  pushl $0xFFFFFF54
  jmp common_interrupt

.global irq172
irq172:
  pushl $0xFFFFFF53
  jmp common_interrupt

.global irq173
irq173:
  pushl $0xFFFFFF52
  jmp common_interrupt

.global irq174
irq174:
  pushl $0xFFFFFF51
  jmp common_interrupt

.global irq175
irq175:
  pushl $0xFFFFFF50
  jmp common_interrupt

.global irq176
irq176:
  pushl $0xFFFFFF4F
  jmp common_interrupt

.global irq177
irq177:
  pushl $0xFFFFFF4E
  jmp common_interrupt

.global irq178
irq178:
  pushl $0xFFFFFF4D
  jmp common_interrupt

.global irq179
irq179:
  pushl $0xFFFFFF4C
  jmp common_interrupt

.global irq180
irq180:
  pushl $0xFFFFFF4B
  jmp common_interrupt

.global irq181
irq181:
  pushl $0xFFFFFF4A
  jmp common_interrupt

.global irq182
irq182:
  pushl $0xFFFFFF49
  jmp common_interrupt

.global irq183
irq183:
  pushl $0xFFFFFF48
  jmp common_interrupt

.global irq184
irq184:
  pushl $0xFFFFFF47
  jmp common_interrupt

.global irq185
irq185:
  pushl $0xFFFFFF46
  jmp common_interrupt

.global irq186
irq186:
  pushl $0xFFFFFF45
  jmp common_interrupt

.global irq187
irq187:
  pushl $0xFFFFFF44
  jmp common_interrupt

.global irq188
irq188:
  pushl $0xFFFFFF43
  jmp common_interrupt

.global irq189
irq189:
  pushl $0xFFFFFF42
  jmp common_interrupt

.global irq190
irq190:
  pushl $0xFFFFFF41
  jmp common_interrupt

.global irq191
irq191:
  pushl $0xFFFFFF40
  jmp common_interrupt

.global irq192
irq192:
  pushl $0xFFFFFF3F
  jmp common_interrupt

.global irq193
irq193:
  pushl $0xFFFFFF3E
  jmp common_interrupt

.global irq194
irq194:
  pushl $0xFFFFFF3D
  jmp common_interrupt

.global irq195
irq195:
  pushl $0xFFFFFF3C
  jmp common_interrupt

.global irq196
irq196:
  pushl $0xFFFFFF3B
  jmp common_interrupt

.global irq197
irq197:
  pushl $0xFFFFFF3A
  jmp common_interrupt

.global irq198
irq198:
  pushl $0xFFFFFF39
  jmp common_interrupt

.global irq199
irq199:
  pushl $0xFFFFFF38
  jmp common_interrupt

.global irq200
irq200:
  pushl $0xFFFFFF37
  jmp common_interrupt

.global irq201
irq201:
  pushl $0xFFFFFF36
  jmp common_interrupt

.global irq202
irq202:
  pushl $0xFFFFFF35
  jmp common_interrupt

.global irq203
irq203:
  pushl $0xFFFFFF34
  jmp common_interrupt

.global irq204
irq204:
  pushl $0xFFFFFF33
  jmp common_interrupt

.global irq205
irq205:
  pushl $0xFFFFFF32
  jmp common_interrupt

.global irq206
irq206:
  pushl $0xFFFFFF31
  jmp common_interrupt

.global irq207
irq207:
  pushl $0xFFFFFF30
  jmp common_interrupt

.global irq208
irq208:
  pushl $0xFFFFFF2F
  jmp common_interrupt

.global irq209
irq209:
  pushl $0xFFFFFF2E
  jmp common_interrupt

.global irq210
irq210:
  pushl $0xFFFFFF2D
  jmp common_interrupt

.global irq211
irq211:
  pushl $0xFFFFFF2C
  jmp common_interrupt

.global irq212
irq212:
  pushl $0xFFFFFF2B
  jmp common_interrupt

.global irq213
irq213:
  pushl $0xFFFFFF2A
  jmp common_interrupt

.global irq214
irq214:
  pushl $0xFFFFFF29
  jmp common_interrupt

.global irq215
irq215:
  pushl $0xFFFFFF28
  jmp common_interrupt

.global irq216
irq216:
  pushl $0xFFFFFF27
  jmp common_interrupt

.global irq217
irq217:
  pushl $0xFFFFFF26
  jmp common_interrupt

.global irq218
irq218:
  pushl $0xFFFFFF25
  jmp common_interrupt

.global irq219
irq219:
  pushl $0xFFFFFF24
  jmp common_interrupt

.global irq220
irq220:
  pushl $0xFFFFFF23
  jmp common_interrupt

.global irq221
irq221:
  pushl $0xFFFFFF22
  jmp common_interrupt

.global irq222
irq222:
  pushl $0xFFFFFF21
  jmp common_interrupt

.global irq223
irq223:
  pushl $0xFFFFFF20
  jmp common_interrupt

.global irq224
irq224:
  pushl $0xFFFFFF1F
  jmp common_interrupt

.global irq225
irq225:
  pushl $0xFFFFFF1E
  jmp common_interrupt

.global irq226
irq226:
  pushl $0xFFFFFF1D
  jmp common_interrupt

.global irq227
irq227:
  pushl $0xFFFFFF1C
  jmp common_interrupt

.global irq228
irq228:
  pushl $0xFFFFFF1B
  jmp common_interrupt

.global irq229
irq229:
  pushl $0xFFFFFF1A
  jmp common_interrupt

.global irq230
irq230:
  pushl $0xFFFFFF19
  jmp common_interrupt

.global irq231
irq231:
  pushl $0xFFFFFF18
  jmp common_interrupt

.global irq232
irq232:
  pushl $0xFFFFFF17
  jmp common_interrupt

.global irq233
irq233:
  pushl $0xFFFFFF16
  jmp common_interrupt

.global irq234
irq234:
  pushl $0xFFFFFF15
  jmp common_interrupt

.global irq235
irq235:
  pushl $0xFFFFFF14
  jmp common_interrupt

.global irq236
irq236:
  pushl $0xFFFFFF13
  jmp common_interrupt

.global irq237
irq237:
  pushl $0xFFFFFF12
  jmp common_interrupt

.global irq238
irq238:
  pushl $0xFFFFFF11
  jmp common_interrupt

.global irq239
irq239:
  pushl $0xFFFFFF10
  jmp common_interrupt

.global irq240
irq240:
  pushl $0xFFFFFF0F
  jmp common_interrupt

.global irq241
irq241:
  pushl $0xFFFFFF0E
  jmp common_interrupt

.global irq242
irq242:
  pushl $0xFFFFFF0D
  jmp common_interrupt

.global irq243
irq243:
  pushl $0xFFFFFF0C
  jmp common_interrupt

.global irq244
irq244:
  pushl $0xFFFFFF0B
  jmp common_interrupt

.global irq245
irq245:
  pushl $0xFFFFFF0A
  jmp common_interrupt

.global irq246
irq246:
  pushl $0xFFFFFF09
  jmp common_interrupt

.global irq247
irq247:
  pushl $0xFFFFFF08
  jmp common_interrupt

.global irq248
irq248:
  pushl $0xFFFFFF07
  jmp common_interrupt

.global irq249
irq249:
  pushl $0xFFFFFF06
  jmp common_interrupt

.global irq250
irq250:
  pushl $0xFFFFFF05
  jmp common_interrupt

.global irq251
irq251:
  pushl $0xFFFFFF04
  jmp common_interrupt

.global irq252
irq252:
  pushl $0xFFFFFF03
  jmp common_interrupt

.global irq253
irq253:
  pushl $0xFFFFFF02
  jmp common_interrupt

.global irq254
irq254:
  pushl $0xFFFFFF01
  jmp common_interrupt

.global irq255
irq255:
  pushl $0xFFFFFF00
  jmp common_interrupt
