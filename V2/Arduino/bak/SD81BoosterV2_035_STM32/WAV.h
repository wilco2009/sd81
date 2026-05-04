uint64_t wavsize = 0;
SdFile wav_file;

void load_wav_buffer(){
  //Load samples into the 8-bit dacBuffer
  long bytes_left = wavsize - playing_pos;
  #ifdef LOG3
  sprintf_P(str1, PSTR("BYTES LEFT: %lu, BUFFER SIZE: %d"), bytes_left, DAC_BUFFER_SIZE);
  Serial.println(str1);
  #endif
  if (bytes_left > DAC_BUFFER_SIZE){
    wav_file.seekSet(playing_pos);
    wav_file.read((void*) DAC_work_buffer+DAC_work_buffer_count, DAC_BUFFER_SIZE);
    #ifdef LOG3
    showHex(DAC_work_buffer);
    #endif
    playing_pos+= DAC_BUFFER_SIZE;
    DAC_work_buffer_count+= DAC_BUFFER_SIZE;
  } else {
    wav_file.seekSet(playing_pos);
    uint32_t result = fonema_file.read((void*) DAC_work_buffer+DAC_work_buffer_count, bytes_left);
    #ifdef LOG3
    Serial.println(result);
    showHex(DAC_work_buffer);
    #endif
    playing_pos+= bytes_left;
    DAC_work_buffer_count+= bytes_left;
  }
  DAC_feed();
}

void playWavFile(char* filename){
  if (playing_wav) wav_file.close();
  wav_file.open(filename);
  if (!sd.exists(filename)) log_0("file %s does not exists.", filename);
  if (!wav_file.isOpen()) log_0("Error opening %s.", filename);
  wavsize = wav_file.fileSize();
  playing_pos = 44; // skip header
  DAC_Init(DAC_VOICE);
  playing_wav = true;
//  while (playing_pos < wavsize){
    log_3("playing_wav %lu",playing_pos);
    load_wav_buffer();
//  }
//  DAC_Init(DAC_AY);
//  wav_file.close();
}
