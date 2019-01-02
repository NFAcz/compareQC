/**
  QCTools search
  Copyright (C) 2018 Kryštof Pešek

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include <fstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <math.h>

using namespace std;
using namespace boost;


char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
  char ** itr = std::find(begin, end, option);
  if (itr != end && ++itr != end)
  {
    return *itr;
  }
  return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return std::find(begin, end, option) != end;
}

static bool is_match(const std::string& text, const std::string& pattern)
{
  return std::string::npos != text.find(pattern);
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);

  while (std::getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }

  return tokens;
}

std::vector<float> load(const char *filename){

  std::ifstream file(filename, std::ios_base::in | std::ios_base::binary);
  boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
  inbuf.push(boost::iostreams::gzip_decompressor());
  inbuf.push(file);

  std::istream instream(&inbuf);

  float Y;
  std::string line;
  std::vector<float> tmp;

  while(std::getline(instream, line)) {
    if(is_match(line,"lavfi.signalstats.YAVG")){
      std::vector<string> ttmp = split(line,'\"');
      Y = float(strtof(ttmp[3].c_str(),NULL));
      tmp.push_back(Y);
    }
  }
  file.close();

  return tmp;
}

int main(int argc, char** argv) {

  if(cmdOptionExists(argv, argv+argc, "-h"))
  {
    std::cerr << "Usage: " << argv[0] << " -a <gzipped QCTools xml input file A> " << std::endl << "-b <gzipped QCTools xml input file B>" << std::endl << "-t <allowed THRESHOLD i.e. 5.0>" << std::endl;
  }

  char * filename1 = getCmdOption(argv, argv + argc, "-a");
  char * filename2 = getCmdOption(argv, argv + argc, "-b");
  char * thresh = getCmdOption(argv, argv + argc, "-t");

  float THRESHOLD = 15;
  int JUMP = 4;
  int something = 0;

  if(thresh){
    std::string ts(thresh);
    THRESHOLD = float(strtof(ts.c_str(),NULL));
  }


  if (filename1 && filename2){
    std::cout << "loading file A: " << filename1 << std::endl;
    std::vector<float> a = load(filename1);
    std::cout << "loaded " << a.size() << " frames" << std::endl;


    std::cout << "loading file B: " << filename2 << std::endl;
    std::vector<float> b = load(filename2);
    std::cout << "loaded " << b.size() << " frames" << std::endl;

    std::cout << "comparing..." << std::endl;

    if(a.size()>b.size()){
      std::cerr << "Error: input file A should be shorter than B file" << std::endl;
      return 1;
    }

    for(int i = 0 ; i < b.size()-a.size();i++){

      float avg = 0;

      for(int ii = 0 ; ii < a.size();ii+=JUMP){
        avg = avg + (fabs(b[ii+i]-a[ii]));
      }

      if(avg==0.0){
        something = 1;
        std::cout << "complete MATCH! file " << filename1 << " and " << filename2 << " are the same @ frame " << i << std::endl; 
        return 0;
      }

      if(avg<(THRESHOLD*a.size()/((float)JUMP))){
        something = 1;
        std::cout << "close match rated: " << (avg/(a.size()/((float)JUMP))) << " @ frame " << i << std::endl; 
      }
    }

  }else{
    std::cerr << "Usage: " << argv[0] << " -a <gzipped QCTools xml input file A> " << std::endl << "-b <gzipped QCTools xml input file B>" << std::endl << "-t <allowed THRESHOLD i.e. 5.0>" << std::endl;
    return 1;
  }

  if(something==0){
    std::cout << "no match found" << std::endl;
    return 1;
  }

  return 0;

}
