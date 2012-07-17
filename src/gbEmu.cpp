/**
 * This file is part of gbemu.
 * Copyright (C) 2008, 2009 Jean-Michel FROUIN
 * Website : http://frouin.me
 */

#include <iostream>
#include <cstdlib>
#include "gameboy.h"

//History moved to def.h

int main (int argc, char** argv)
{
  int Test=0xFFFF;
  unsigned char Temp=false;
  std::cout << NAME << " " << VERSION << "\n";
  if(argc!=2)
  {
    std::cout << "Need a rom file\n";
    return EXIT_FAILURE;
  }
  std::cout << "Loading : " << argv[1] << '\n';
  Hardware::XGameboy GB;
  std::cout << "\n\n\n\n\n\n\n";
  GB.Run(argv[1]);
  std::cout << "LOG - EntryPoint: OK :) \n";
  return EXIT_SUCCESS;
}
