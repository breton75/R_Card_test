#ifndef SV_CRC_H
#define SV_CRC_H


class SvCRC32
{
public:
  SvCRC32() 
  {
    // инициализируем таблицу расчёта Crc32
    unsigned long crc;//переменная 32 бита = 4 байтам
    
    for (int i = 0; i < 256; i++) {
      
      crc = i;
      
      for (int j = 0; j < 8; j++)
        crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
      
      crc_table[i] = crc;
      
    }
  }
  
  unsigned long calc(unsigned char *buf, unsigned long len)
  {
    unsigned long crc = 0xFFFFFFFFUL;
    
    while (len--)
      crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    
    return crc ^ 0xFFFFFFFFUL;
  }
  
  unsigned long crc_table[256];
  
//private:
  
  
};

#endif // SV_CRC_H
