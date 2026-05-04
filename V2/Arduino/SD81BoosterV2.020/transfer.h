
/*
 * You can test this over your USB port using lrzsz: `stty -F /dev/ttyUSB0 4800 && sx -vaX /path/to/send/file > /dev/ttyUSB0 < /dev/ttyUSB0`
 * If you want to try CRC_XMODEM then add a o flag to the sx command (-vaoX)
 */
bool process_block(void *blk_id, byte *data, size_t dataSize) {
  byte id = *((byte *) blk_id);
  for(int i = 0; i < dataSize; ++i) {
    //do stuff with the recieved data
  }

  //return false to stop the transfer
  return true;
}
