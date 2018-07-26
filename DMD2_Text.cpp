/*
 DMD2 Text Handling Functions.

 Copyright (C) 2014 Freetronics, Inc. (info <at> freetronics <dot> com)

 Updated by Angus Gratton, based on DMD by Marc Alexander & FTOLED_Text.cpp
 from the FTOLED library.

---

 This program is free software: you can redistribute it and/or modify it under the terms
 of the version 3 GNU General Public License as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.
*/

#include "DMD2.h"

void DMDFrame::selectFont(const uint8_t* font)
{
  this->font = (uint8_t *)font;
}

int DMDFrame::drawChar(const int x, const int y, const char letter, DMDGraphicsMode mode, const int overByte, const uint8_t *font)
{
  if(!font)
    font = this->font;
  if(x >= (int)width || y >= height)
    return -1;

  struct FontHeader header;
  memcpy_P(&header, (void*)font, sizeof(FontHeader));

  DMDGraphicsMode invertedMode = inverseMode(mode);

  uint16_t c = letter;
  if (c == ' ') {
    int charWide = charWidth(' ');
    this->drawFilledBox(x, y, x + charWide, y + header.height, invertedMode);
    return charWide;
  }
  if (overByte > 0) {
    c = letter + 255;
  }
  uint8_t width = 0;
  uint8_t bytes = (header.height + 7) / 8;
  uint16_t index = 0;

  if (c < header.firstChar || c >= (header.firstChar + header.charCount))
    return 0;
  c -= header.firstChar;

  if (header.size == 0) {
    // zero length is flag indicating fixed width font (array does not contain width data entries)
    width = header.fixedWidth;
    index = sizeof(FontHeader) + c * bytes * width;
  } else {
    // variable width font, read width data, to get the index
    for (uint8_t i = 0; i < c; i++) {
      index += pgm_read_byte(font + sizeof(FontHeader) + i);
    }
    index = index * bytes + header.charCount + sizeof(FontHeader);
    width = pgm_read_byte(font + sizeof(FontHeader) + c);
  }
  if (x < -width || y < -header.height)
    return width;
    
  bool inverse = false;
  if (mode == GRAPHICS_INVERSE) {
      inverse = true;
  }

  // last but not least, draw the character
  for (uint8_t j = 0; j < width; j++) { // Width
    for (uint8_t i = bytes - 1; i < 254; i--) { // Vertical Bytes
      uint8_t data = pgm_read_byte(font + index + j + (i * width));
      int offset = (i * 8);
      if ((i == bytes - 1) && bytes > 1) {
        offset = header.height - 8;
      }
      for (uint8_t k = 0; k < 8; k++) { // Vertical bits
        if ((offset+k >= i*8) && (offset+k <= header.height)) {
          if (data & (1 << k)) {
              if(inverse) {
                setPixel(x + j, y + offset + k, GRAPHICS_OFF);
              } else {
                setPixel(x + j, y + offset + k, GRAPHICS_ON);
              }
          } else {
              if(inverse) {
                  setPixel(x + j, y + offset + k, GRAPHICS_ON);
              } else {
                  setPixel(x + j, y + offset + k, GRAPHICS_OFF);
              }
          }
        }
      }
    }
  }
  return width;
}

// Generic drawString implementation for various kinds of strings
template <class StrType> __attribute__((always_inline)) inline void _drawString(DMDFrame *dmd, int x, int y, StrType str, DMDGraphicsMode mode, const uint8_t *font)
{
  struct FontHeader header;
  memcpy_P(&header, font, sizeof(FontHeader));

  if (y+header.height<0)
    return;

  DMDGraphicsMode invertedMode = inverseMode(mode);
  int strWidth = 0;
  if(x > 0)
    dmd->drawLine(x-1 , y, x-1 , y + header.height - 1, invertedMode);

  char c;
  for(int idx = 0; c = str[idx], c != 0; idx++) {
    if(c == '\n') { // Newline
      strWidth = 0;
      y = y - header.height - 1;
    }
    else {
      int charWide = 0;
      if (c == 255) {
        idx++;
        c = str[idx];
        charWide = dmd->drawChar(x+strWidth, y, c, mode, 1);
      } else {
        charWide = dmd->drawChar(x+strWidth, y, c, mode, 0);
      }
      
      if (charWide > 0) {
        strWidth += charWide ;
        dmd->drawLine(x + strWidth , y, x + strWidth , y + header.height-1, invertedMode);
        strWidth++;
      } else if (charWide < 0) {
        return;
      }
    }
  }
}

// Generic stringWidth implementation for various kinds of strings
template <class StrType> __attribute__((always_inline)) inline unsigned int _stringWidth(DMDFrame *dmd, const uint8_t *font, StrType str)
{
  unsigned int width = 0;
  char c;
  int idx;
  for(idx = 0; c = str[idx], c != 0; idx++) {
    int cwidth = dmd->charWidth(c);
    if(cwidth > 0)
      width += cwidth + 1;
  }
  if(width) {
    width--;
  }
  return width;
}


#if defined(__AVR__) || defined (ESP8266)
// Small wrapper class to allow indexing of progmem strings via [] (should be inlined out of the actual implementation)
class _FlashStringWrapper {
  const char *str;
public:
  _FlashStringWrapper(const char * flstr) : str(flstr) { }
  inline char operator[](unsigned int index) {
    return pgm_read_byte(str + index);
  }
};

void DMDFrame::drawString_P(int x, int y, const char *flashStr, DMDGraphicsMode mode, const uint8_t *font)
{
  if(!font)
    font = this->font;
  if(x >= (int)width || y >= height)
    return;
  _FlashStringWrapper wrapper(flashStr);
  _drawString(this, x, y, wrapper, mode, font);
}

unsigned int DMDFrame::stringWidth_P(const char *flashStr, const uint8_t *font)
{
  if(!font)
    font = this->font;
  _FlashStringWrapper wrapper(flashStr);
  return _stringWidth(this, font, wrapper);
}

#endif

void DMDFrame::drawString(int x, int y, const char *bChars, DMDGraphicsMode mode, const uint8_t *font)
{
  if(!font)
    font = this->font;
  if (x >= (int)width || y >= height)
    return;
  _drawString(this, x, y, bChars, mode, font);
}

void DMDFrame::drawString(int x, int y, const String &str, DMDGraphicsMode mode, const uint8_t *font)
{
  if(!font)
    font = this->font;
  if (x >= (int)width || y >= height)
    return;
  _drawString(this, x, y, str, mode, font);
}

//Find the width of a character
int DMDFrame::charWidth(const char letter, const uint8_t *font)
{
  struct FontHeader header;
  memcpy_P(&header, (void*)this->font, sizeof(FontHeader));

  if(!font)
    font = this->font;

  if(letter == ' ') {
    // if the letter is a space then return the font's fixedWidth
    // (set as the 'width' field in New Font dialog in GLCDCreator.)
    return header.fixedWidth;
  }

  if((uint8_t)letter < header.firstChar || (uint8_t)letter >= (header.firstChar + header.charCount)) {
    return 0;
  }

  if(header.size == 0) {
    // zero length is flag indicating fixed width font (array does not contain width data entries)
    return header.fixedWidth;
  }

  // variable width font, read width data for character
  return pgm_read_byte(this->font + sizeof(FontHeader) + letter - header.firstChar);
}

unsigned int DMDFrame::stringWidth(const char *bChars, const uint8_t *font)
{
  if(!font)
    font = this->font;
  return _stringWidth(this, font, bChars);
}

unsigned int DMDFrame::stringWidth(const String &str, const uint8_t *font)
{
  if(!font)
    font = this->font;
  return _stringWidth(this, font, str);
}


char* DMDFrame::ConvertStringToArrayChar(String x, bool display) {
  int length = x.length();
  char *resultConvert = new char[length + 1]; // or
  int len = 0;
  if (display) {
    Serial.println("\n" + x);
  }
  char *y = new char[length + 1]; // or
  strcpy(y, x.c_str());
  if (display) {
    while (*y) {
      if (*y == ' ') {
        Serial.println("  ");
      } else {
        Serial.print((int)*y);
        Serial.print(" ");
      }
      *y++;
    }
  }
  else {
    // println("");
    for (int i = 0; i < length; i++) {
      char c = 0;
      if (y[i] < 128) {
        // Serial.print(y[i]);
        resultConvert[len++] = y[i];
        
      } else if (y[i] < 225) {
        String tg = "_";
        if (y[i] == 195 && i < length - 1) {
          if (y[i+1] == 161) { tg = "á"; c = 128; }
          else if (y[i+1] == 160) { tg = "à"; c = 129; }
          else if (y[i+1] == 163) { tg = "ã"; c = 132; }

          else if (y[i+1] == 162) { tg = "â"; c = 139; }

          else if (y[i+1] == 169) { tg = "é"; c = 146; }
          else if (y[i+1] == 168) { tg = "è"; c = 147; }

          else if (y[i+1] == 170) { tg = "ê"; c = 151; }

          else if (y[i+1] == 173) { tg = "í"; c = 157; }
          else if (y[i+1] == 172) { tg = "ì"; c = 158; }

          else if (y[i+1] == 179) { tg = "ó"; c = 162; }
          else if (y[i+1] == 178) { tg = "ò"; c = 163; }
          else if (y[i+1] == 181) { tg = "õ"; c = 166; }

          else if (y[i+1] == 180) { tg = "ô"; c = 167; }

          else if (y[i+1] == 186) { tg = "ú"; c = 179; }
          else if (y[i+1] == 185) { tg = "ù"; c = 180; }

          else if (y[i+1] == 189) { tg = "ý"; c = 190; }

          else if (y[i+1] == 129) { tg = "Á"; c = 195; }
          else if (y[i+1] == 128) { tg = "À"; c = 196; }
          else if (y[i+1] == 131) { tg = "Ã"; c = 199; }

          else if (y[i+1] == 130) { tg = "Â"; c = 206; }

          else if (y[i+1] == 137) { tg = "É"; c = 213; }
          else if (y[i+1] == 136) { tg = "È"; c = 214; }


          else if (y[i+1] == 138) { tg = "Ê"; c = 218; }

          else if (y[i+1] == 141) { tg = "Í"; c = 224; }
          else if (y[i+1] == 140) { tg = "Ì"; c = 225; }

          else if (y[i+1] == 147) { tg = "Ó"; c = 229; }
          else if (y[i+1] == 146) { tg = "Ò"; c = 230; }
          else if (y[i+1] == 149) { tg = "Õ"; c = 233; }

          else if (y[i+1] == 148) { tg = "Ô"; c = 234; }

          else if (y[i+1] == 154) { tg = "Ú"; c = 246; }
          else if (y[i+1] == 153) { tg = "Ù"; c = 247; }

          else if (y[i+1] == 157) { tg = "Ý"; c = 258 - 255; resultConvert[len++] = 255;}


        } else if (y[i] == 196 && i < length - 1) {
          if (y[i+1] == 131) { tg = "ă"; c = 133; }

          else if (y[i+1] == 169) { tg = "ĩ"; c = 161; }

          else if (y[i+1] == 145) { tg = "đ"; c = 145; }

          else if (y[i+1] == 130) { tg = "Ă"; c = 200; }

          else if (y[i+1] == 144) { tg = "Đ"; c = 212; }

          else if (y[i+1] == 168) { tg = "Ĩ"; c = 228; }


        } else if (y[i] == 197 && i < length - 1) {
          if (y[i+1] == 169) { tg = "ũ"; c = 183; }

          else if (y[i+1] == 168) { tg = "Ũ"; c = 250; }
          
        } else if (y[i] == 198 && i < length - 1) {
          if (y[i+1] == 161) { tg = "ơ"; c = 173; }

          else if (y[i+1] == 176) { tg = "ư"; c = 184; }

          else if (y[i+1] == 160) { tg = "Ơ"; c = 240; }

          else if (y[i+1] == 175) { tg = "Ư"; c = 251; }

        }
        resultConvert[len++] = c;
        // Serial.print(tg);
        i = i + 1;
      } else {
        String tg = "=";
        if (y[i] == 225 && i < length - 1) {
          if (y[i+1] == 186 && i + 1 < length - 1) {
            if (y[i+2] == 161) { tg = "ạ"; c = 130; }
            else if (y[i+2] == 163) { tg = "ả"; c = 131; }

            else if (y[i+2] == 165) { tg = "ấ"; c = 140; }
            else if (y[i+2] == 167) { tg = "ầ"; c = 141; }
            else if (y[i+2] == 173) { tg = "ậ"; c = 142; }
            else if (y[i+2] == 169) { tg = "ẩ"; c = 143; }
            else if (y[i+2] == 171) { tg = "ẫ"; c = 144; }

            else if (y[i+2] == 175) { tg = "ắ"; c = 134; }
            else if (y[i+2] == 177) { tg = "ằ"; c = 135; }
            else if (y[i+2] == 183) { tg = "ặ"; c = 136; }
            else if (y[i+2] == 179) { tg = "ẳ"; c = 137; }
            else if (y[i+2] == 181) { tg = "ẵ"; c = 138; }

            else if (y[i+2] == 185) { tg = "ẹ"; c = 148; }
            else if (y[i+2] == 187) { tg = "ẻ"; c = 149; }
            else if (y[i+2] == 189) { tg = "ẽ"; c = 150; }
            else if (y[i+2] == 191) { tg = "ế"; c = 152; }

            else if (y[i+2] == 160) { tg = "Ạ"; c = 197; }
            else if (y[i+2] == 162) { tg = "Ả"; c = 198; }

            else if (y[i+2] == 174) { tg = "Ắ"; c = 201; }
            else if (y[i+2] == 176) { tg = "Ằ"; c = 202; }
            else if (y[i+2] == 182) { tg = "Ặ"; c = 203; }
            else if (y[i+2] == 178) { tg = "Ẳ"; c = 204; }
            else if (y[i+2] == 180) { tg = "Ẵ"; c = 205; }

            else if (y[i+2] == 164) { tg = "Ấ"; c = 207; }
            else if (y[i+2] == 166) { tg = "Ầ"; c = 208; }
            else if (y[i+2] == 172) { tg = "Ậ"; c = 209; }
            else if (y[i+2] == 168) { tg = "Ẩ"; c = 210; }
            else if (y[i+2] == 170) { tg = "Ẫ"; c = 211; }

            else if (y[i+2] == 184) { tg = "Ẹ"; c = 215; }
            else if (y[i+2] == 186) { tg = "Ẻ"; c = 216; }
            else if (y[i+2] == 188) { tg = "Ẽ"; c = 217; }

            else if (y[i+2] == 190) { tg = "Ế"; c = 219; }



          } else if (y[i+1] == 187 && i + 1 < length - 1) {
            
            if (y[i+2] == 129) { tg = "ề"; c = 153; }
            else if (y[i+2] == 135) { tg = "ệ"; c = 154; }
            else if (y[i+2] == 131) { tg = "ể"; c = 155; }
            else if (y[i+2] == 133) { tg = "ễ"; c = 156; }

            else if (y[i+2] == 139) { tg = "ị"; c = 159; }
            else if (y[i+2] == 137) { tg = "ỉ"; c = 160; }

            else if (y[i+2] == 141) { tg = "ọ"; c = 164; }
            else if (y[i+2] == 143) { tg = "ỏ"; c = 165; }

            else if (y[i+2] == 145) { tg = "ố"; c = 168; }
            else if (y[i+2] == 147) { tg = "ồ"; c = 169; }
            else if (y[i+2] == 153) { tg = "ộ"; c = 170; }
            else if (y[i+2] == 149) { tg = "ổ"; c = 171; }
            else if (y[i+2] == 151) { tg = "ỗ"; c = 172; }

            else if (y[i+2] == 155) { tg = "ớ"; c = 174; }
            else if (y[i+2] == 157) { tg = "ờ"; c = 175; }
            else if (y[i+2] == 163) { tg = "ợ"; c = 176; }
            else if (y[i+2] == 159) { tg = "ở"; c = 177; }
            else if (y[i+2] == 161) { tg = "ỡ"; c = 178; }

            else if (y[i+2] == 165) { tg = "ụ"; c = 181; }
            else if (y[i+2] == 167) { tg = "ủ"; c = 182; }

            else if (y[i+2] == 169) { tg = "ứ"; c = 185; }
            else if (y[i+2] == 171) { tg = "ừ"; c = 186; }
            else if (y[i+2] == 177) { tg = "ự"; c = 187; }
            else if (y[i+2] == 173) { tg = "ử"; c = 188; }
            else if (y[i+2] == 175) { tg = "ữ"; c = 189; }

            else if (y[i+2] == 179) { tg = "ỳ"; c = 191; }
            else if (y[i+2] == 181) { tg = "ỵ"; c = 192; }
            else if (y[i+2] == 183) { tg = "ỷ"; c = 193; }
            else if (y[i+2] == 185) { tg = "ỹ"; c = 194; }

            else if (y[i+2] == 128) { tg = "Ề"; c = 220; }
            else if (y[i+2] == 134) { tg = "Ệ"; c = 221; }
            else if (y[i+2] == 130) { tg = "Ể"; c = 222; }
            else if (y[i+2] == 132) { tg = "Ễ"; c = 223; }

            else if (y[i+2] == 138) { tg = "Ị"; c = 226; }
            else if (y[i+2] == 136) { tg = "Ỉ"; c = 227; }

            else if (y[i+2] == 140) { tg = "Ọ"; c = 231; }
            else if (y[i+2] == 142) { tg = "Ỏ"; c = 232; }

            else if (y[i+2] == 144) { tg = "Ố"; c = 235; }
            else if (y[i+2] == 146) { tg = "Ồ"; c = 236; }
            else if (y[i+2] == 152) { tg = "Ộ"; c = 237; }
            else if (y[i+2] == 148) { tg = "Ổ"; c = 238; }
            else if (y[i+2] == 150) { tg = "Ỗ"; c = 239; }

            else if (y[i+2] == 154) { tg = "Ớ"; c = 241; }
            else if (y[i+2] == 156) { tg = "Ờ"; c = 242; }
            else if (y[i+2] == 162) { tg = "Ợ"; c = 243; }
            else if (y[i+2] == 158) { tg = "Ở"; c = 244; }
            else if (y[i+2] == 160) { tg = "Ỡ"; c = 245; }

            else if (y[i+2] == 164) { tg = "Ụ"; c = 248; }
            else if (y[i+2] == 166) { tg = "Ủ"; c = 249; }

            else if (y[i+2] == 168) { tg = "Ứ"; c = 252; }
            else if (y[i+2] == 170) { tg = "Ừ"; c = 253; }
            else if (y[i+2] == 176) { tg = "Ự"; c = 254; }
            else if (y[i+2] == 172) { tg = "Ử"; c = 256 - 255; resultConvert[len++] = 255;}
            else if (y[i+2] == 174) { tg = "Ữ"; c = 257 - 255; resultConvert[len++] = 255;}

            else if (y[i+2] == 178) { tg = "Ỳ"; c = 259 - 255; resultConvert[len++] = 255;}
            else if (y[i+2] == 180) { tg = "Ỵ"; c = 260 - 255; resultConvert[len++] = 255;}
            else if (y[i+2] == 182) { tg = "Ỷ"; c = 261 - 255; resultConvert[len++] = 255;}
            else if (y[i+2] == 184) { tg = "Ỹ"; c = 262 - 255; resultConvert[len++] = 255;}



          }
          resultConvert[len++] = c;       
        }
        // Serial.print(tg);
        i = i + 2;
      }
    }
    // show("\nlen:" + String(len) + "\n");
  }
  resultConvert[len] = '\0';
  return resultConvert;
}
