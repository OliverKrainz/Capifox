set talk off
clear
str = "(C) OK>Soft Oliver Kocks"

crc = 0
for l = 1 to len(str)
  crc = crc + asc(substr(str, l, 1))
endfor
?crc