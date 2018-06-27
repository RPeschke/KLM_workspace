//compile independently with: g++ -o ./bin/tx_ethparse1_ck ./src/tx_ethparse1_ck.cpp `root-config --cflags --glibs` -Wall
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <vector>


typedef std::pair<int,int> interval_t;

const int NSAMP         = 32*4;
const int MEMORY_DEPTH  = 512;
const int NASICS        = 10;
const int NCHS          = 16;
bool DecodeTPGflag;

using namespace std;

class MBevent {
    public:
    int    EvtNum;
    int    AddNum;
    int    WrAddNum;
    int    Wctime; // ???
    int    ASIC;
    int    Sample[16][128];
    int    PeakVal[16];
    int    PeakTime[16];
    // note: Window information is only allocated 3 bits, so
    //       window number is fully specified by sample number

    MBevent() {
        EvtNum                                     = -1;
        AddNum                                     = -1;
        WrAddNum                                   = -1;
        Wctime                                     = -1;
        ASIC                                       = -1;
        for (int i=0; i<16; i++) PeakVal [i]       = 9999;
        for (int i=0; i<16; i++) PeakTime [i]      = -1;
        for (int i=0; i<16; i++) {
            for (int j=0; j<128; j++) Sample[i][j] = 9999;
        }

    }


friend std::ostream& operator<<(std::ostream& out, const MBevent& evt){
        out <<      evt.EvtNum      << " ";
        out <<      evt.AddNum      << " ";
        out <<      evt.WrAddNum    << " ";
        out <<      evt.Wctime      << " ";
        out <<      evt.ASIC        << " ";
        for (int i=0; i<16; i++) {
            out <<  evt.PeakTime[i] << " ";
            out <<  evt.PeakVal[i]  << " ";
            for (int j=0; j<128; j++) out << evt.Sample[i][j] << " ";
        }
        out << "\n";

        return out;
    }
};


 //----FIND BUFFER POSITION FOR START OF PACKAGE----//

int  FIND_BUFFER_POSITION_FOR_START_OF_PACKAGE(const  unsigned char* buff,  int pos1,long size){
    while (
        (pos1 < (size-16))
          && (!((buff[pos1]=='w' && buff[pos1+1]=='a' && buff[pos1+2]=='v' && buff[pos1+3]=='e')) // for OutMode==0 //ex. wave-->-------\    |pos1    --------         pos2\       (all utf 8-bit)
          &&  !((buff[pos1]=='c' && buff[pos1+1]=='h' && buff[pos1+2]=='r' && buff[pos1+3]=='g'))) // for OutMode==1//sdf0k\Uffffffffdf\Uffffffffet\Uffffffffk\Uffffffffwq\Uffffffff\Uffffffffjwave\Uffffffff\Uffffffff\Uffffffff\Uffffffffu\Uffffffff57\Uffffffff\Ufffffffffm\Uffffffff/\Uffffffff08\Uffffffff\Uffffffff\Uffffffff\Uffffffffhrg=\Uffffffff3\Uffffffff\Uffffffff\Uffffffff1\Uffffffff\Uffffffffhn\Uffffffffhk\Uffffffff
            ) {
            pos1++;
    }

    return pos1;
}

interval_t Find_package_in_buffer(const  unsigned char* buff, const interval_t interval_v, long size){
    int pos1 = interval_v.first;
    pos1 = FIND_BUFFER_POSITION_FOR_START_OF_PACKAGE(buff,pos1,size);
    pos1 += 4; // Increment both place holders past package markers identifed above.

    int pos2 = FIND_BUFFER_POSITION_FOR_START_OF_PACKAGE(buff,pos1,size);

    return interval_t(pos1,pos2);
}
bool is_valid_package(const  unsigned char* buff, const interval_t interval_v){
    int pos1 = interval_v.first;
    return (buff[pos1]==0xFE && buff[pos1+4]==0xCF && (buff[pos1+8]&0xF0)==0xD0);
}
bool end_of_package_marker(const unsigned char* buff,int pos1){
     // End of package marked by hex words "FACEFACE = \Uffffffff?
    return ((buff[pos1] == 0xFA) && (buff[pos1+1] == 0xCE) && (buff[pos1+2] == 0xFA) && (buff[pos1+3] == 0xCE));
}

//----CONVERT DATA FROM 8-BIT BUFFER TO 32-BIT BUFFER----//
interval_t CONVERT_DATA_FROM_8_BIT_BUFFER_TO_32_BIT_BUFFER(const unsigned char* buff,const interval_t interval_v,  std::vector<unsigned int>&  buffer_uint){
    buffer_uint.clear();
    int posRead=0;
    int pos1 = interval_v.first;
    while (
    (pos1< interval_v.second)  && !end_of_package_marker(buff,pos1)) {
        if (posRead%4==0) { // Fill waveform buffer.
            buffer_uint.push_back(
                ((unsigned int)buff[pos1+0])<<24 | ((unsigned int)buff[pos1+1])<<16 | ((unsigned int)buff[pos1+2])<<8 | ((unsigned int)buff[pos1+3])
            );

        }
        posRead++;
        pos1++; // When at end of while-loop, pos1 should be either at the word FACEFACE or at start of next package?
    }


    return interval_t(pos1,interval_v.second);
}

void PRINT_PSEUDO_STATUS_BAR_TO_TERMINAL(int eventCounter){

            //----PRINT PSEUDO STATUS BAR TO TERMINAL----//
            //if (eventCounter == 1) cout << "Passing package of " << wbuflen <<" 32-bit words to waveform buffer\n";
            if (eventCounter%1 == 0) cout << "." << flush;
            if (eventCounter%80 == 0) cout << "<---" << eventCounter << "\n";
}


//----CHECK FOR FEATURE EXTRACTION --  INDICATED BY "CB" VALUE----//
void CHECK_FOR_FEATURE_EXTRACTION(const std::vector<unsigned int>& buffer_uint, const int offset,const int count, MBevent& evt){
    if ((buffer_uint[offset+count]& 0xFF000000)==0xCB000000) { // note: current usage is 31 bits
        int chanNum                 = (0x00F00000 & buffer_uint[offset+count])>>20;
        evt.PeakTime[chanNum]  = buffer_uint[offset+count]>>12 & 0x7F; // 7 bits for sample number
        evt.PeakVal[chanNum]   = (buffer_uint[offset+count] & 0x0FFF); // 12 bit ADC count for peak
    }
}

//----CHECK FOR SAMPLE -- INDICATED BY "BD" VALUE----//
bool CHECK_FOR_SAMPLE(const std::vector<unsigned int>& buffer_uint, const int offset,const int count,const int chanNum, const int window,  MBevent& evt){
    // current usage is 31 bits
    if ((buffer_uint[offset+count] & 0xFF000000) == 0xBD000000) {
        const int sampNum     = ((buffer_uint[offset+count]>>12) & 0x0000001F)+window*32; // word #3 & up: read 5 bits
        int BDval   =  (buffer_uint[offset+count]>>24) & 0x000000FF; // word #3 & up: read 8 bits (top 8 bits)
        // make sure info is still valid before extracting samples---must pass these four conditions
        if (evt.AddNum >= MEMORY_DEPTH  || evt.ASIC > NASICS || sampNum >= NSAMP || BDval != 0xBD) {
            cout << "INVALID SAMPLE INFO, SKIPPING "    << "\n"
                 << "Loop counter: "    << count        << "\n"
                 << "Address number: "  << evt.AddNum  << "\n"
                 << "ASIC number: "     << evt.ASIC    << "\n"
                 << "Sample number: "   << sampNum      << "\n"
                 << "BD value: "        << BDval        << "\n";

            return true;
        }
        else {
            evt.Sample[chanNum][sampNum] = ((buffer_uint[offset+count]) & 0x00000FFF); // word #3 & up: read 12 bits (bottom 12 bits)
        }
    }
    return false;
}

 //----CHECK FOR TRIGGER BITS -- INDICATED BY "CA" WORD FOLLOWED BY "C7" WORD----//
void CHECK_FOR_TRIGGER_BITS(const std::vector<unsigned int>& buffer_uint,const int offset,const int count, std::ostream& out){
    const int n_ca_max=32; // total of 64 hex words, 32 each of "CAXXXXXX" and "C7XXXXXX"
    int n_ca                         = 0;
    unsigned int CA_val[n_ca_max]    = {0};
    unsigned int CA_raw1[n_ca_max]   = {0};
    unsigned int CA_raw2[n_ca_max]   = {0};
    unsigned int CA_win[n_ca_max]    = {0};
    unsigned int CA_tbs[n_ca_max][4] = {{0}};
    // note: (CA)(23 downto 17)(7 downto 0) and (C7)(23 downto 20) are only stored in raw's
    if ((buffer_uint[offset+count] & 0xFF000000) == 0xCA000000) {
        if ((buffer_uint[offset+count+1] & 0xFF000000) == 0xC7000000) {
            if (n_ca < n_ca_max) {
                CA_raw1[n_ca]   =  buffer_uint[offset+count  ]; // store "CA" word just in case
                CA_raw2[n_ca]   =  buffer_uint[offset+count+1]; // store "C7" word just in case
                CA_val[n_ca]    =  buffer_uint[offset+count+1] & 0x000FFFFF; // (C7) 19 downto 0 // redundant
                CA_tbs[n_ca][0] = (buffer_uint[offset+count+1]>>0 ) & 0x0000001F; // (C7) 4 downto 0
                CA_tbs[n_ca][1] = (buffer_uint[offset+count+1]>>5 ) & 0x0000001F; // (C7) 9 downto 5
                CA_tbs[n_ca][2] = (buffer_uint[offset+count+1]>>10) & 0x0000001F; // (C7) 14 downto 10
                CA_tbs[n_ca][3] = (buffer_uint[offset+count+1]>>15) & 0x0000001F; // (C7) 19 downto 15
                CA_win[n_ca]    = (buffer_uint[offset+count  ] & 0x0001FF00) >> 8; // (CA) 16 downto 8
                out << CA_raw1[n_ca]   << "\t";
                out << CA_raw2[n_ca]   << "\t";
                out << CA_val[n_ca]    << "\t";
                out << CA_win[n_ca]    << "\t";
                out << CA_tbs[n_ca][0] << "\t";
                out << CA_tbs[n_ca][1] << "\t";
                out << CA_tbs[n_ca][2] << "\t";
                out << CA_tbs[n_ca][3] << "\n";
            }
            n_ca++; //useless??
        }
    }
 }
void extract_package_header(const std::vector<unsigned int>& buffer_uint, MBevent& evt){
        //----FOR EACH 32-bit WORD, EXTRACT DATA USING BIT-WISE & OPERATORS----//
    evt.AddNum     =   (buffer_uint[0])      & 0x000001FF ;   // word #0: read 5 bits
    evt.WrAddNum   =   (buffer_uint[0]>>9)   & 0x000001FF ;   // word #0: read 5 bits
    evt.ASIC       =  ((buffer_uint[0]>>20)  & 0x0000000F)-1; // word #0: read 4 bits
    evt.EvtNum     =   (buffer_uint[1])      & 0x00FFFFFF ;   // word #1: read 24 bits
    evt.Wctime     =   (buffer_uint[2])      & 0x0FFFFFFF ;   // word #2: read 28 bits
}


//----LOOP OVER CURRENT PACKAGE AND EXTRACT DATA----//
void extract_package_body(const std::vector<unsigned int>&  buffer_uint,  MBevent& evt,std::ostream& out){
    const size_t offset     = 3;
    for(size_t count = 0 ; count < buffer_uint.size() ; ++count){
        int chanNum    = (buffer_uint[offset+count]>>19) & 0x0000000F; // word #3 & up: read 4 bits
        int window     = (buffer_uint[offset+count]>>17) & 0x00000003; // word #3 & up: read 2 bits

        //----CHECK FOR SAMPLE -- INDICATED BY "BD" VALUE----//
        // current usage is 31 bits
        if(CHECK_FOR_SAMPLE(buffer_uint,offset,count,chanNum,window,evt)){
            continue;
        }
        CHECK_FOR_FEATURE_EXTRACTION(buffer_uint,offset,count,evt);
        CHECK_FOR_TRIGGER_BITS(buffer_uint,offset,count,out);

    }//END OF EXTRACT DATA FROM CURRENT PACKAGE
}
//-----------------------------------------------------//
//                   PROCESS BUFFER
//-----------------------------------------------------//
void processBuffer(const unsigned char* buff,const long size, const char* waveformOutfile,const char* trigBitOutfile,const int OutMode) {

    if (size<100) {
        cout << "\n\nWARNING: Packet size only: " << size << " bytes. ---> Skipping packet.\n\n";
        return;
    }

    ofstream outfile(waveformOutfile, ofstream::out | ofstream::app); // data in form of ascii (8-bit) characters // app opt. writes to end.


    ofstream CAoutfile(trigBitOutfile, ofstream::out | ofstream::app);



    MBevent evt;


     //---- EXTRACT PACKAGES FROM BUFFER ----//
    std::vector<unsigned int> buffer_uint(65536);

    int eventCounter = 1; // for printing progress to terminal
    interval_t packet_interval(0,0);


    while (packet_interval.first < (size-16)) {
        packet_interval = Find_package_in_buffer(buff,packet_interval,size);


        if (!is_valid_package(buff,packet_interval)) { // We dont have a package.
           continue;
        }

        packet_interval = CONVERT_DATA_FROM_8_BIT_BUFFER_TO_32_BIT_BUFFER( buff,packet_interval,buffer_uint);


        PRINT_PSEUDO_STATUS_BAR_TO_TERMINAL(eventCounter++);

        extract_package_header(buffer_uint,evt);
        extract_package_body(buffer_uint,evt,CAoutfile);


        //----WRITE PARSED PACKAGE TO FILE (ONE LINE PER EVENT)----//
        outfile << evt;


    }//END OF ALL PACKAGES
    cout << "<---" << eventCounter-1 << "\n";


}//END PROCESS BUFFER

long   GET_LENGTH_OF_FILE(std::ifstream& infile){
    infile.seekg (0, infile.end);       // Seek to a position offset 0 from end of infile stream.
    long size_in_bytes = infile.tellg(); // Return position of get-pointer.
    infile.seekg (0, infile.beg);       // Move get-pointer back to beginning of file stream buffer.
    return size_in_bytes;
}


std::vector<char> read_file_to_buffer(const char* inFileName){
    //---- DEFINE INPUT FILE AND PARSING ----//
    ifstream infile(inFileName, ifstream::in | ifstream::binary); // data in form of ascii (8-bit) characters  // Open file for reading in binary.
    if (infile.fail()) {
        cout << "\n\nError opening input file, exiting\n\n";
        return std::vector<char>();
    }


     //---- READ DATA TO BUFFER ----//
    std::vector<char> buffer(GET_LENGTH_OF_FILE(infile)); // Declare memory

    cout << "Reading " << buffer.size() << " bytes... ";
    infile.read (buffer.data(), buffer.size());
    if (infile) cout << "all characters read successfully.\n";
    else cout << "\n\nerror: only " << infile.gcount() << " could be read\n\n";
    return buffer;
}
//--------------------------------------------------------//
//                  MAIN
//--------------------------------------------------------//
int main(int argc, char* argv[]) {

    //------------ USAGE ------------//
    if (argc != 5 && argc != 6 ) {
        cout << "wrong number of arguments:\n";
        cout << "Usage ./tx_ethparse1 <input filename> <waveformOutfile> <trigBitoutfile> <OutMode> [options: -TPG]\n";
        return 0; //          0            1                   2                 3            4
    }

    DecodeTPGflag = false;
    if (argc == 6) {
        if (0 == strcmp(argv[5], "-TPG")) DecodeTPGflag = true;
        else {
            cout << "Usage: ./tx_ethparse1 <input filename> <waveformOutfile> <trigBitoutfile> <OutMode> [options: -TPG]\n";
            return 0;
        }
    }

    char* inputFileName   = argv[1]; // outdir/data.dat
    char* waveformOutfile = argv[2]; // outdir/KLMS . . . /
    char* trigBitOutfile  = argv[3];
    int   OutMode         = atoi(argv[4]);

    std::vector<char> buffer = read_file_to_buffer(inputFileName);

    if(buffer.empty()){
        return -1; // reurn an error code to the command line
    }
    unsigned char* Ubuff = (unsigned char*)buffer.data(); // point to ascii buffer and cast it as unsigned buffer
    processBuffer(Ubuff, buffer.size(), waveformOutfile, trigBitOutfile, OutMode);


    return 0;
}
