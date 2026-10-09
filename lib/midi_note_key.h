// Copyright 2026 Zack Scholl, GPLv3.0
#ifndef LIB_MIDI_NOTE_KEY_H
#define LIB_MIDI_NOTE_KEY_H

// Included after the playback globals and audio callback state. Other devices
// retain their existing defaults; -DMIDI_NOTE_KEY=0 disables slice notes.
#ifndef MIDI_NOTE_KEY
#ifdef INCLUDE_ZEPTOCORE
#define MIDI_NOTE_KEY 1
#else
#define MIDI_NOTE_KEY 0
#endif
#endif

int fx_mapping[16] = {12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3 };

int midi_note_mapping[20] = {
  0, 5, 10, 15,
  1, 6, 11, 16, 
  2, 7, 12, 17, 
  3, 8, 13, 18, 
  4, 9, 14, 19
};


void midi_note_off(int note) {
  if(note < 21){
    midi_buttons[midi_note_mapping[note]] = false;
    return;
  }
  if(note > 23 && note < 40){   // midi notes
    sf->fx_active[fx_mapping[note-24]] = false;
    if(fx_mapping[note-24] == fx_button){
      fx_button = -1;
    }
    update_fx(fx_mapping[note-24]);
    return;
  }

  if(note>19 && note<24){
    return;
  }
  // Slice notes are triggers, not gates. Release never changes transport/mute.
  (void)note;
}

void midi_note_on(int note, int velocity) {
#if MIDI_NOTE_KEY == 1
  // Velocity zero is Note Off, including on the serial MIDI input path.
  if (note < 0 || note > 127 || velocity <= 0 || velocity > 127 ||
      playback_stopped || do_stop_playback || !audio_media_timer_allowed() ||
      !fil_is_open || fil_current_change || fil_current_change_force ||
      do_open_file_ready) {
    return;
  }
  if(note < 20){   // midi notes
    midi_buttons[midi_note_mapping[note]] = true;
    return;
  }
  if(note>19 && note<24){
    return;
  }
  if(note > 23 && note < 40){   // midi notes
    sf->fx_active[fx_mapping[note-24]] = true;
    fx_button = fx_mapping[note-24];
    update_fx(fx_mapping[note-24]);
    return;
  }

  uint8_t bank = sel_bank_cur;
  uint8_t sample = sel_sample_cur;
  if (bank >= 16 || !banks[bank] || !banks[bank]->sample ||
      sample >= banks[bank]->num_samples) {
    return;
  }
  SampleInfo *sound = banks[bank]->sample[sample].snd[FILEZERO];
  if (!sound || !sound->slice_num || !sound->slice_start || !sound->slice_stop) {
    return;
  }
  uint8_t slice = note % sound->slice_num;
  key_do_jump_to_slice(slice, slice % 16);
#else
  (void)note;
  (void)velocity;
#endif
}

#endif
