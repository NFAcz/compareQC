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
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <math.h>


using namespace std;
using namespace boost;

float FPS = -1;


/* colors */

const std::string red("\033[0;31m");
const std::string green("\033[1;32m");
const std::string yellow("\033[1;33m");
const std::string cyan("\033[0;36m");
const std::string magenta("\033[0;35m");
const std::string reset("\033[0m");

/*
   command line argument parser
   */
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
  char ** itr = std::find(begin, end, option);
  if (itr != end && ++itr != end)
  {
    return *itr;
  }
  return 0;
}

/*
   command line argument check
   */
bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return std::find(begin, end, option) != end;
}

/*
   string match routine (true if contains string)
   */
static bool is_match(const std::string& text, const std::string& pattern)
{
  return std::string::npos != text.find(pattern);
}


/*
   spltting string by delimiter
   */
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



/*
 * loading and parsing prepared uncopressed txt files
 * formatted [faster]
 * Y (float)
 * U (float)
 * V (float)
 */
std::vector<float> loadFromTxt(const char *filename, const int jump){

    std:ifstream instream(filename);
    float Y;
    std::string line;
    std::vector<float> tmp;
    int counter = 0;
    int frms = 0;
    while(std::getline(instream, line)){
      if(counter%3==0){
        if(frms%jump==0){
        Y = float(strtof(line.c_str(),NULL));
        tmp.push_back(Y);
        }
        frms++;
      }
      counter++;
    }
    instream.close();

    FPS = 25.0;

    return tmp;
}


/*
   loading and parsing values from gziiped source into scalable vector array
   */
std::vector<float> load(const char *filename, const int jump){

  std::ifstream file(filename, std::ios_base::in | std::ios_base::binary);
  boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
  inbuf.push(boost::iostreams::gzip_decompressor());
  inbuf.push(file);

  std::istream instream(&inbuf);

  float Y;
  std::string line;
  std::vector<float> tmp;

  int counter = 0;

  while(std::getline(instream, line)) {

    if(is_match(line,"lavfi.signalstats.YAVG")){
      if(counter%jump==0){
        std::vector<string> ttmp = split(line,'\"');
        Y = float(strtof(ttmp[3].c_str(),NULL));
        tmp.push_back(Y);
      }
      counter++;
    }

    if(is_match(line,"stream index=\"0")){
      std::vector<string> ttmp = split(line,'\"');
      std::vector<string> fracc = split(ttmp[41],'/'); // some older versions the index is 39!
      float frac = float(strtof(fracc[0].c_str(),NULL)) / float(strtof(fracc[1].c_str(),NULL)) ;
      if(FPS!=-1 && FPS!=frac){
        std::cout << "warning source FPS differs!" << std::endl;
      }
      FPS = frac;

    }
  }
  file.close();

  return tmp;
}


/*
   print help function
   */
void help(string program){
  std::cerr << "Usage: " << program << " -a <gzipped QCTools xml input file A> " << "-b <gzipped QCTools xml input file B>" << std::endl << "[ opt -t <allowed THRESHOLD i.e. 5.0> -s <speedup - jump N samples> ]" << std::endl;
}

/*
   main loop goes here
   */
int main(int argc, char** argv) {

  if(cmdOptionExists(argv, argv+argc, "-h"))
  {
    help(argv[0]);
    return 0;
  }

  char * filename1 = getCmdOption(argv, argv + argc, "-a");
  char * filename2 = getCmdOption(argv, argv + argc, "-b");
  char * thresh = getCmdOption(argv, argv + argc, "-t");
  char * jump = getCmdOption(argv, argv + argc, "-s");
  char * verbose = getCmdOption(argv, argv + argc, "-v");

  float THRESHOLD = 7.5;
  int JUMP = 1;
  int something = 0;

  float lowest = 1024.0;
  float index = 0;
  string TCindex;

  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  int frames = 0;

  bool DEBUG = false;

  if(verbose){
    DEBUG = true;
  }

  // parse -t
  if(thresh){
    std::string ts(thresh);
    THRESHOLD = float(strtof(ts.c_str(),NULL));
  }

  //parse -s
  if(jump){
    std::string ts(jump);
    JUMP = int(strtof(ts.c_str(),NULL));
  }

  //parse -a and -b (no file existance check so far)
  if (filename1 && filename2){
    std::vector<float> a,b;
    if(DEBUG)
      std::cout << "loading file A: " << filename1 << std::endl;

    if(is_match(filename1,".xml.gz")){
      a = load(filename1,JUMP);
    }else if(is_match(filename1,".txt")){
      a = loadFromTxt(filename1,JUMP);
    }

    if(DEBUG)
      std::cout << "loaded " << a.size() << " frames of total " << (JUMP * a.size()) << " (skip "<< JUMP <<")" << std::endl;


    if(DEBUG)
      std::cout << "loading file B: " << filename2 << std::endl;

    if(is_match(filename2,".xml.gz")){
      b = load(filename2,JUMP);
    }else if(is_match(filename2,".txt")){
      b = loadFromTxt(filename2,JUMP);
    }

    if(DEBUG)
      std::cout << "loaded " << b.size() << " frames of total " << (JUMP * b.size()) << " (skip "<< JUMP <<")" << std::endl;

    if(DEBUG)
      std::cout << "detected FPS: " << FPS << std::endl;

    if(DEBUG)
      std::cout << "comparing..." << std::endl;

    if(a.size()>b.size()){
      std::cerr << yellow << "Note: input file A should be shorter than B file ("<<filename2<<")" << reset << std::endl;
      return 1;
    }

    // sample comparation main loop
    for(int i = 0 ; i < b.size() - a.size() ; i++){


      // timecode estimation
      //
      if(frames * JUMP >= FPS){
        frames = 0;
        seconds++;
      }

      if(seconds > 59){
        seconds = 0;
        minutes++;
      }

      if(minutes > 59){
        minutes = 0;
        hours++;
      }

      stringstream tc;
      tc << setfill('0') << std::setw(2) << hours;
      tc << ":";
      tc << setfill('0') << std::setw(2) << minutes;
      tc << ":";
      tc << setfill('0') << std::setw(2) << seconds;
      tc << ":";
      tc << setfill('0') << std::setw(2) << frames;
      string TC = tc.str();



      // weird placement but if match is perfect it needs to be broken here
      if(something==2){
        break;
      }

      // if the diff of first sample of B is whitin threshold do seek more samples
      if(fabs(b[i]-a[0])<THRESHOLD){

        float avg = 0;

        // compute average diffenrence
        int cnt = 0;

        for(int ii = 0 ; ii < a.size() ; ii++){
          avg = avg + ( fabs( b[ii+i] - a[ii] ));
          cnt++;
        }
        avg = avg / cnt;

        // this means perfect match
        if(avg==0.0){
          something = 2;
          index = i * JUMP;
          TCindex = TC;
        }

        // record if avg in range
        if(avg < THRESHOLD && something !=2 ){
          something = 1;
          index = i * JUMP;
          TCindex = TC;
        }

        // collect the best fitting record
        if(avg < lowest){
          lowest = avg;
          index = i * JUMP;
          TCindex = TC;
        }

      }

      frames+=1;
    }

  }else{
    help(argv[0]);
    return 0;
  }

  if(something==0){
    if(DEBUG)
    std::cout << "NO match, threshold is " << THRESHOLD << ", best: " << yellow << lowest << reset << " (" << filename2 << ")" << std::endl;
    //std::cout << "FR: " << index << std::endl;
    //std::cout << "TC: " << TCindex << std::endl;
    return 1;
  }

  if(something==1){
    std::cout << green << "close MATCH found" << reset << " whitin threshold of " << THRESHOLD << ", it scores: " << yellow << lowest << reset << std::endl;
    std::cout << filename2 << std::endl;
    std::cout << "FR: " << index << std::endl;
    std::cout << "TC: " << TCindex << std::endl;
  }

  if(something==2){
    std::cout << green << "complete MATCH!" << reset << " file " << filename1 << " and " << filename2 << " are the same @:" << std::endl;
    std::cout << "FR: " << index << std::endl;
    std::cout << "TC: " << TCindex << std::endl;
    return 0;
  }
  return 0;

}
