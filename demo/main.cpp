#include <bitset>
#include <boost/program_options.hpp>
#include <iostream>

typedef int FXPT2DOT30;

typedef struct {
  FXPT2DOT30 ciexyzX;
  FXPT2DOT30 ciexyzY;
  FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef struct {
  CIEXYZ ciexyzRed;
  CIEXYZ ciexyzGreen;
  CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
  unsigned short bfType;
  unsigned int bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int bfOffBits;
} BITMAPFILEHEADER;

// bitmap info header v1
#pragma pack(1)
typedef struct tagBITMAPINFOHEADER {
  unsigned int biSize;
  unsigned int biWidth;
  unsigned int biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned int biCompression;
  unsigned int biSizeImage;
  unsigned int biXPelsPerMeter;
  unsigned int biYPelsPerMeter;
  unsigned int biClrUsed;
  unsigned int biClrImportant;
} BITMAPINFOHEADER;
// v2
typedef struct tagBITMAPINFOHEADER2 {
  unsigned int biRedMask;
  unsigned int biGreenMask;
  unsigned int biBlueMask;
} BITMAPINFOHEADER2;
// v3
typedef struct {
  unsigned int biAlphaMask;
} BITMAPINFOHEADER3;
// v4
typedef struct {
  unsigned int biCSType;
  CIEXYZTRIPLE biEndpoints;
  unsigned int biGammaRed;
  unsigned int biGammaGreen;
  unsigned int biGammaBlue;
} BITMAPINFOHEADER4;

// v5
typedef struct {
  unsigned int biIntent;
  unsigned int biProfileData;
  unsigned int biProfileSize;
  unsigned int biReserved;
} BITMAPINFOHEADER5;

// rgb quad
#pragma pack(1)
typedef struct tagRGBQUAD {
  unsigned char rgbBlue;
  unsigned char rgbGreen;
  unsigned char rgbRed;
  unsigned char rgbReserved;
} RGBQUAD;

namespace opt = boost::program_options;

int main(int argc, char *argv[]) {
  opt::options_description desc(
      "Использовние:\n--crypt --input [PATH] --output [PATH] --message "
      "[PATH]\nor\n--decrypt --input [PATH] --message [PATH]\nor\n--info "
      "--input [PATH]\nAll options");
  desc.add_options()
        ("crypt", "означает, что программа работает в режиме сокрытия информации")
        ("decrypt", "означает, что программа работает в режиме извлечения информации")
        ("info", "означает, что программа работает в режиме получения заголовков изображения")
        ("input", opt::value<std::string>()->value_name("=PATH"), "полный путь до оригинального изображения")
        ("output", opt::value<std::string>()->value_name("=PATH"), "полный путь до файла с сокрытой информацией")
        ("message",opt::value<std::string>()->value_name("=PATH"),"полный путь до файла с информацией, которую необходимо скрыть в изображении или извлечь из изображения")
        ("help", "показать справку");

  try {
    opt::variables_map vm;
    opt::store(opt::parse_command_line(argc, argv, desc), vm);
    opt::notify(vm);
    if (vm.count("help") || vm.empty()) {
      std::cout << desc << std::endl;
      return 1;
    }
    if (vm.count("crypt") && vm.count("decrypt")) {
      std::cout << desc << std::endl;
      return 1;
    }
    if (vm.count("crypt")) {
      std::string inputPATH = vm["input"].as<std::string>();
      std::string outputPATH = vm["output"].as<std::string>();
      std::string messagePATH = vm["message"].as<std::string>();
      FILE *image = fopen(inputPATH.c_str(), "rb");
      FILE *text = fopen(messagePATH.c_str(), "rb");
      // seek to end of file
      fseek(text, 0, SEEK_END);
      // get current file position which is end from seek
      size_t size1 = ftell(text);
      std::string ss;
      // allocate string space and set length
      ss.resize(size1);
      // go back to beginning of file for read
      rewind(text);
      // read 1*size bytes from sfile into ss
      fread(&ss[0], 1, size1, text);
      // close the file
      fclose(text);
      std::string ssbit;
      for (char s : ss) {
        std::bitset<CHAR_BIT> l(s);
        ssbit += l.to_string();
      }
      std::bitset<32> size_of_crypt_bit(ssbit.length());
      tagBITMAPFILEHEADER *file_header = new tagBITMAPFILEHEADER();
      fread(&file_header->bfType, sizeof(unsigned char), 2, image);
      fread(&file_header->bfSize, sizeof(unsigned char), 4, image);
      fread(&file_header->bfReserved1, sizeof(unsigned char), 2, image);
      fread(&file_header->bfReserved2, sizeof(unsigned char), 2, image);
      fread(&file_header->bfOffBits, sizeof(unsigned char), 4, image);
      if (file_header->bfType != 0x4D42) {
        std::cout << "Error: '" << inputPATH << "' is not BMP file."
                  << std::endl;
        return 0;
      }
      //Считываю infoheader
      tagBITMAPINFOHEADER *file_infoheader = new tagBITMAPINFOHEADER();
      tagBITMAPINFOHEADER2 *file_infoheaderV2 = new tagBITMAPINFOHEADER2();
      BITMAPINFOHEADER3 *file_infoheaderV3 = new BITMAPINFOHEADER3();
      BITMAPINFOHEADER4 *file_infoheaderV4 = new BITMAPINFOHEADER4();
      BITMAPINFOHEADER5 *file_infoheaderV5 = new BITMAPINFOHEADER5();
      fread(&file_infoheader->biSize, sizeof(unsigned char), 4, image);
      if (file_infoheader->biSize >= 40) {
        fread(&file_infoheader->biWidth, sizeof(unsigned char), 4, image);
        fread(&file_infoheader->biHeight, sizeof(unsigned char), 4, image);
        fread(&file_infoheader->biPlanes, sizeof(unsigned char), 2, image);
        fread(&file_infoheader->biBitCount, sizeof(unsigned char), 2, image);
        fread(&file_infoheader->biCompression, sizeof(unsigned char), 4, image);
        fread(&file_infoheader->biSizeImage, sizeof(unsigned char), 4, image);
        fread(&file_infoheader->biXPelsPerMeter, sizeof(unsigned char), 4,
              image);
        fread(&file_infoheader->biYPelsPerMeter, sizeof(unsigned char), 4,
              image);
        fread(&file_infoheader->biClrUsed, sizeof(unsigned char), 4, image);
        fread(&file_infoheader->biClrImportant, sizeof(unsigned char), 4,
              image);
      }
      if (file_infoheader->biSize >= 52) {
        fread(&file_infoheaderV2->biRedMask, sizeof(unsigned char), 4, image);
        fread(&file_infoheaderV2->biGreenMask, sizeof(unsigned char), 4, image);
        fread(&file_infoheaderV2->biBlueMask, sizeof(unsigned char), 4, image);
      }
      if (file_infoheader->biSize >= 56) {
        fread(&file_infoheaderV3->biAlphaMask, sizeof(unsigned char), 4, image);
      }
      if (file_infoheader->biSize >= 108) {
        fread(&file_infoheaderV4->biCSType, sizeof(unsigned char), 4, image);
        fread(&file_infoheaderV4->biEndpoints, sizeof(CIEXYZTRIPLE), 1, image);
        fread(&file_infoheaderV4->biGammaRed, sizeof(unsigned char), 4, image);
        fread(&file_infoheaderV4->biGammaGreen, sizeof(unsigned char), 4,
              image);
        fread(&file_infoheaderV4->biGammaBlue, sizeof(unsigned char), 4, image);
      }
      if (file_infoheader->biSize >= 124) {
        fread(&file_infoheaderV5->biIntent, sizeof(unsigned char), 4, image);
        fread(&file_infoheaderV5->biProfileData, sizeof(unsigned char), 4,
              image);
        fread(&file_infoheaderV5->biProfileSize, sizeof(unsigned char), 4,
              image);
        fread(&file_infoheaderV5->biReserved, sizeof(unsigned char), 4, image);
      }

      if (file_infoheader->biSize < 52){
        delete file_infoheaderV2; delete file_infoheaderV3; delete file_infoheaderV4; delete file_infoheaderV5;
      } else if (file_infoheader->biSize < 56){
        delete file_infoheaderV3; delete file_infoheaderV4; delete file_infoheaderV5;
      } else if (file_infoheader->biSize < 104){
         delete file_infoheaderV4; delete file_infoheaderV5;
      }else if (file_infoheader->biSize < 124){
         delete file_infoheaderV5;
      }
      //Считываю пиксели
      int size =
          ceil(float(file_header->bfSize - 14 - file_infoheader->biSize) / 4);
      tagRGBQUAD *data = new tagRGBQUAD[size];
      if (size < ssbit.length()) {
        std::cout << "The input image is too small to hide the message"
                  << std::endl;
        return 0;
      }
      for (int i = 0; i < size; ++i) {
        fread(&data[i], sizeof(tagRGBQUAD), 1, image);
      }
      fclose(image);
      for (int i = 0; i < size_of_crypt_bit.size(); ++i) {
        data[i].rgbRed = data[i].rgbRed >> 1;
        data[i].rgbRed = data[i].rgbRed << 1;
        data[i].rgbRed = data[i].rgbRed | size_of_crypt_bit[i];
      }
      for (int i = size_of_crypt_bit.size();
           i < ssbit.length() + size_of_crypt_bit.size(); ++i) {
        data[i].rgbRed = data[i].rgbRed >> 1;
        data[i].rgbRed = data[i].rgbRed << 1;
        data[i].rgbRed = data[i].rgbRed | ssbit[i - size_of_crypt_bit.size()];
      }
      FILE *modified_image = fopen(outputPATH.c_str(), "wb");
      if (file_infoheader->biSize >= 40) {
        fwrite(file_header, sizeof(tagBITMAPFILEHEADER), 1, modified_image);
        fwrite(file_infoheader, sizeof(tagBITMAPINFOHEADER), 1, modified_image);
      }
      if (file_infoheader->biSize >= 52) {
        fwrite(file_infoheaderV2, sizeof(BITMAPINFOHEADER2), 1, modified_image);
      }
      if (file_infoheader->biSize >= 56) {
        fwrite(file_infoheaderV3, sizeof(BITMAPINFOHEADER3), 1, modified_image);
      }
      if (file_infoheader->biSize >= 108) {
        fwrite(file_infoheaderV4, sizeof(BITMAPINFOHEADER4), 1, modified_image);
      }
      if (file_infoheader->biSize >= 124) {
        fwrite(file_infoheaderV5, sizeof(BITMAPINFOHEADER5), 1, modified_image);
      }
      fwrite(data, sizeof(tagRGBQUAD), size, modified_image);
      fclose(modified_image);
    }
    if (vm.count("decrypt")) {
      std::string inputPATH = vm["input"].as<std::string>();
      std::string messagePATH = vm["message"].as<std::string>();
      FILE *imagetodecrypt = fopen(inputPATH.c_str(), "rb");
      tagBITMAPFILEHEADER *file_header2 = new tagBITMAPFILEHEADER();
      tagBITMAPINFOHEADER2 *file_infoheader2V2 = new tagBITMAPINFOHEADER2();
      BITMAPINFOHEADER3 *file_infoheader2V3 = new BITMAPINFOHEADER3();
      BITMAPINFOHEADER4 *file_infoheader2V4 = new BITMAPINFOHEADER4();
      BITMAPINFOHEADER5 *file_infoheader2V5 = new BITMAPINFOHEADER5();
      // reading
      fread(&file_header2->bfType, sizeof(unsigned char), 2, imagetodecrypt);
      fread(&file_header2->bfSize, sizeof(unsigned char), 4, imagetodecrypt);
      fread(&file_header2->bfReserved1, sizeof(unsigned char), 2,
            imagetodecrypt);
      fread(&file_header2->bfReserved2, sizeof(unsigned char), 2,
            imagetodecrypt);
      fread(&file_header2->bfOffBits, sizeof(unsigned char), 4, imagetodecrypt);
      tagBITMAPINFOHEADER *file_infoheader2 = new tagBITMAPINFOHEADER();
      fread(&file_infoheader2->biSize, sizeof(unsigned char), 4,
            imagetodecrypt);
      if (file_infoheader2->biSize >= 40) {
        fread(&file_infoheader2->biWidth, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2->biHeight, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2->biPlanes, sizeof(unsigned char), 2,
              imagetodecrypt);
        fread(&file_infoheader2->biBitCount, sizeof(unsigned char), 2,
              imagetodecrypt);
        fread(&file_infoheader2->biCompression, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2->biSizeImage, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2->biXPelsPerMeter, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2->biYPelsPerMeter, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2->biClrUsed, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2->biClrImportant, sizeof(unsigned char), 4,
              imagetodecrypt);
      }
      if (file_infoheader2->biSize >= 52) {
        fread(&file_infoheader2V2->biRedMask, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2V2->biGreenMask, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2V2->biBlueMask, sizeof(unsigned char), 4,
              imagetodecrypt);
      }
      if (file_infoheader2->biSize >= 56) {
        fread(&file_infoheader2V3->biAlphaMask, sizeof(unsigned char), 4,
              imagetodecrypt);
      }
      if (file_infoheader2->biSize >= 108) {
        fread(&file_infoheader2V4->biCSType, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2V4->biEndpoints, sizeof(CIEXYZTRIPLE), 1,
              imagetodecrypt);
        fread(&file_infoheader2V4->biGammaRed, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2V4->biGammaGreen, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2V4->biGammaBlue, sizeof(unsigned char), 4,
              imagetodecrypt);
      }
      if (file_infoheader2->biSize >= 124) {
        fread(&file_infoheader2V5->biIntent, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2V5->biProfileData, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2V5->biProfileSize, sizeof(unsigned char), 4,
              imagetodecrypt);
        fread(&file_infoheader2V5->biReserved, sizeof(unsigned char), 4,
              imagetodecrypt);
      }
      int size2 =
          ceil(float(file_header2->bfSize - 14 - file_infoheader2->biSize) / 4);
      tagRGBQUAD *data2 = new tagRGBQUAD[size2];
      for (int i = 0; i < size2; ++i) {
        fread(&data2[i], sizeof(tagRGBQUAD), 1, imagetodecrypt);
      }
      // decrypting
      std::string size_decrypt_bit;
      for (int i = 0; i < 32; ++i) {
        size_decrypt_bit += std::to_string(data2[i].rgbRed & 1);
      }
      std::reverse(size_decrypt_bit.begin(), size_decrypt_bit.end());
      auto kaka = std::bitset<32>(size_decrypt_bit.c_str()).to_ullong();
      std::string decrypt_text_bit;
      for (int i = 32; i < 32 + kaka; ++i) {
        decrypt_text_bit += std::to_string(data2[i].rgbRed & 1);
      }
      std::string final;
      for (int i = 0; i < decrypt_text_bit.length();) {
        std::string buf;
        for (int j = 0; j < 8; ++j, ++i) {
          buf += decrypt_text_bit[i];
        }
        final += char(std::bitset<8>(buf.c_str()).to_ullong());
      }
      // writing
      FILE *final_text = fopen(messagePATH.c_str(), "wb");
      fwrite(final.c_str(), final.size(), 1, final_text);
    }
    if (vm.count("info")) {
      std::string inputPATH = vm["input"].as<std::string>();
      FILE *image = fopen(inputPATH.c_str(), "rb");
      tagBITMAPFILEHEADER *file_header = new tagBITMAPFILEHEADER();
      fread(&file_header->bfType, sizeof(unsigned char), 2, image);
      fread(&file_header->bfSize, sizeof(unsigned char), 4, image);
      fread(&file_header->bfReserved1, sizeof(unsigned char), 2, image);
      fread(&file_header->bfReserved2, sizeof(unsigned char), 2, image);
      fread(&file_header->bfOffBits, sizeof(unsigned char), 4, image);
      if (file_header->bfType != 0x4D42) {
        std::cout << "Error: '" << inputPATH << "' is not BMP file."
                  << std::endl;
        return 0;
      }
      tagBITMAPINFOHEADER *file_infoheader = new tagBITMAPINFOHEADER();
      fread(&file_infoheader->biSize, sizeof(unsigned char), 4, image);
      fread(&file_infoheader->biWidth, sizeof(unsigned char), 4, image);
      fread(&file_infoheader->biHeight, sizeof(unsigned char), 4, image);
      fread(&file_infoheader->biPlanes, sizeof(unsigned char), 2, image);
      fread(&file_infoheader->biBitCount, sizeof(unsigned char), 2, image);
      fread(&file_infoheader->biCompression, sizeof(unsigned char), 4, image);
      fread(&file_infoheader->biSizeImage, sizeof(unsigned char), 4, image);
      fread(&file_infoheader->biXPelsPerMeter, sizeof(unsigned char), 4, image);
      fread(&file_infoheader->biYPelsPerMeter, sizeof(unsigned char), 4, image);
      fread(&file_infoheader->biClrUsed, sizeof(unsigned char), 4, image);
      fread(&file_infoheader->biClrImportant, sizeof(unsigned char), 4, image);
      std::cout << "Size: " << file_infoheader->biSize << std::endl
                << "Height: " << file_infoheader->biHeight << std::endl
                << "Width: " << file_infoheader->biWidth << std::endl
                << "Planes: " << file_infoheader->biPlanes << std::endl
                << "Bit Count: " << file_infoheader->biBitCount << std::endl
                << "Compression: " << file_infoheader->biCompression
                << std::endl
                << "Size Image: " << file_infoheader->biSizeImage << std::endl
                << "X Pels Per Meter: " << file_infoheader->biXPelsPerMeter
                << std::endl
                << "Y Pels Per Meter: " << file_infoheader->biYPelsPerMeter
                << std::endl
                << "Clr Used: " << file_infoheader->biClrUsed << std::endl
                << "Clr Important: " << file_infoheader->biClrImportant
                << std::endl;
    }
  } catch (std::exception &e) {
    std::cout << desc << e.what();
  }
}
