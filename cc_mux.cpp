/*
	cc_mux.c: Insert raw closed captions into MPEG files
	Run file without arguments to see usage
	Version 1.2
	McPoodle (mcpoodle43@yahoo.com)
	Part of SCC_TOOLS:
		http://www.geocities.com/mcpoodle43/SCC_TOOLS/DOCS/SCC_TOOLS.HTML
 
	The bits library and much of the code here was lifted from bbMPEG
		and the bbMPEG Tools, by Brent Beyeler, beyeler@home.com
 
	Version History
	1.0 initial release
    1.1 added Top_Field_First and Repeat_First_Field to count_ccgops(),
        changed CCtruncate to CCextra
    1.2 fixed Top_Field_First and Repeat_First_Field use
*/

#include "cc_mux.h"

int main(int argc, char* argv[])
{
	using namespace std;
	// initial variables
	string input1 ("~");
	string input2 ("~");
    string GAfile ("~");
	string MPGinputfile ("~");
	string BINinputfile ("~");
	string MPGoutputfile ("~");
	int analyze = 0;
	int useCCanalysis = 0;

	string arg("");
	int compal; // result of comparison operations
	// process command line arguments
	if (argc == 1) {
		usage();
		return 0;
	}
    for (int i=1; i < argc; i++) {
		arg.assign(argv[i]);
		// cout << "Argument " << i << " is " << arg << endl;
		compal = arg.compare("-a");
		if (compal == 0) {
			if (argc != 4) {
				usage();
				cerr << "Incorrect number of arguments for Syntax 2." << endl;
				return -1;
			}
			analyze = 1;
			i++;
			GAfile.assign(argv[i]);
			continue;
		}
		compal = arg.compare("-g");
		if (compal == 0) {
			if ((i+1) == argc) {
				usage();
				cerr << "GOP analysis file name not provided for Syntax 1." << endl;
				return -1;
			}
			i++;
			GAfile.assign(argv[i]);
			continue;
		}
		compal = input1.compare("~");
		if (compal == 0) {
			input1.assign(arg);
			continue;
		}
		compal = input2.compare("~");
		if (compal == 0) {
			input2.assign(arg);
			continue;
		}
		compal = MPGoutputfile.compare("~");
		if (compal == 0) {
			MPGoutputfile.assign(arg);
			continue;
		}
	}

	/*
	cout << "Input 1: " << input1 << endl;
	cout << "Input 2: " << input2 << endl;
	cout << "Output: " << MPGoutputfile << endl;
	*/

	compal = input1.compare("~");
	if (compal == 0) {
		usage();
		cerr << "No input file(s)!" << endl;
		return -1;
	}
	compal = input2.compare("~");
	if ((!analyze) && (compal == 0)) {
		usage();
		cerr << "Missing an input file for Syntax 1." << endl;
		return -1;
	}

	// examine input files
	basic_string <char>::size_type index;
	string base1, base2, ext1(""), ext2("");
	index = input1.rfind(".");
	if (index != string::npos) {
		base1 = input1.substr(0, index);
		ext1 = input1.substr(index+1, string::npos);
		//cout << "Input 1 split into " << base1 << " + . + " << ext1 << endl;
	}
	index = input2.rfind(".");
	if (index != string::npos) {
		base2 = input2.substr(0, index);
		ext2 = input2.substr(index+1, string::npos);
		//cout << "Input 2 split into " << base2 << " + . + " << ext2 << endl;
	}
	compal = ext1.compare("bin");
	if (compal == 0) {
		BINinputfile = input1;
		MPGinputfile = input2;
		compal = MPGoutputfile.compare("~");
		if (compal == 0)
			MPGoutputfile = base2 + "_CC." + ext2;
	}
	compal = ext2.compare("bin");
	if (compal == 0) {
		BINinputfile = input2;
		MPGinputfile = input1;
		compal = MPGoutputfile.compare("~");
		if (compal == 0)
			MPGoutputfile = base1 + "_CC." + ext1;
	}
	compal = MPGinputfile.compare("~");
	if (compal == 0) {
		MPGinputfile = input1;
	}
	/*
	cout << "GAfile: " << GAfile << endl;	
	cout << "BINinputfile: " << BINinputfile << endl;
	cout << "MPGinputfile: " << MPGinputfile << endl;
	cout << "MPGoutputfile: " << MPGoutputfile << endl;
	*/
	
	if (analyze) {
		cout << "Analyzing MPEG file for CC GOP pattern" << endl;
		count_ccgops(MPGinputfile, GAfile);
		return 0;
	}
	
	compal = BINinputfile.compare("~");
	if (compal == 0) {
		cerr << "No BIN caption input file!" << endl;
		return -1;
	}
	compal = GAfile.compare("~");
	if (compal != 0) {
		useCCanalysis = 1;
		cout << "Analyzing " << GAfile << "." << endl;
		readGA(GAfile);
		if (gopidx == -1) {
			cout << "Unable to get CC GOP analysis from " << GAfile << "." << endl;
			cout << "Resorting to frame GOP analysis" << endl;
			useCCanalysis = 0;
		}
	}
	if (!useCCanalysis) {
		cout << "First pass: Count frames in each GOP..." << endl;
		count_gops(MPGinputfile);
		cout << "Second pass: ";
	} else {
		readGA(GAfile);
	}
	gopidx = -1;
	outfp = NULL;
	bufidx = -1;
	binfp = NULL;
	cout << "Muxing..." << endl;
	mux(MPGinputfile, BINinputfile, MPGoutputfile, useCCanalysis);
	return 0;
}

void usage(void)
{
	using namespace std;
	cout << endl << "CC_MUX Version 1.2" << endl;
	cout << "  Inserts closed captions in raw format into an MPEG file" << endl;
	cout << "    (Video Elementary Stream, not Program Stream)." << endl << endl;
	cout << "  Syntax 1: CC_MUX -g infile1.ga infile2.m2v infile3.bin outfile.m2v" << endl;
    cout << "    -g infile1.ga (OPTIONAL): get CC GOP analysis from infile1.ga" << endl;
    cout << "         (DEFAULT is to use the frame GOP structure of infile2.m2v)" << endl;
	cout << "    infile2.???: MPEG 1 or 2 video file to process (any of various extensions)" << endl;
	cout << "    infile3.bin: Closed Caption file to process, in raw broadcast format" << endl;
	cout << "    outfile.??? (OPTIONAL): name of MPEG file to output" << endl;
	cout << "         (DEFAULT: base name of infile2 plus _CC and same extension)" << endl << endl;
    cout << "  Syntax 2: CC_MUX -a outfile.ga infile.m2v" << endl;
    cout << "    -a outfile.ga: output CC GOP structure to outfile.ga" << endl;
    cout << "    infile.???: MPEG 1 or 2 video file with closed captions to analyze" << endl << endl;
	return;
}

void count_ccgops(std::string MPGinputfile, std::string GAoutputfile) {
	char tmpStr[256];
	unsigned long i, j;
	size_t n;

	using namespace std;
	n = MPGinputfile.copy(tmpStr, 256, 0);
	tmpStr[n] = 0x00;
	init_getbits(tmpStr);
	gopidx = -1;
	do {
		i = getbits(32);
nextpass:
		switch (i) {
			case GROUP_START_CODE:
				gopidx++;
				break;
			case USER_DATA_START_CODE:
				j = getbits(32); // skip closed caption header
				if (j == DVD_CLOSED_CAPTION) {
					i = getbits(8);
					gopcaptions[gopidx] = i;
				}
				break;
			default:
				if ((i >> 8) != PACKET_START_CODE_PREFIX) {
					if (seek_sync(PACKET_START_CODE_PREFIX, 24)) {
						i = 0x00000100 | getbits(8);
						goto nextpass;
					}
					if (end_bs())
						break;
					cout << endl << endl << "Unknown bits in stream, could not resynch" << endl;
					exit(1);
				}
		}
	} while ((i != MPEG_PROGRAM_END_CODE) && (!end_bs()));

	finish_getbits();
	n = GAoutputfile.copy(tmpStr, 256, 0);
	tmpStr[n] = 0x00;
	outfp = fopen(tmpStr, "w");
	if (outfp == 0)
		return;
	fprintf(outfp, "%d\n", gopidx);
	for (i=0; i<=gopidx; i++) {
		fprintf(outfp, "%d\n", gopcaptions[i]);
	}
	fclose(outfp);
	return;
}

void count_gops(std::string MPGinputfile) {
	char tmpStr[256];
	unsigned char CCpattern, CCextra, CCframes, CCextraFields;
	unsigned long i, j;
	size_t n;

	using namespace std;
	n = MPGinputfile.copy(tmpStr, 256, 0);
	tmpStr[n] = 0x00;
	init_getbits(tmpStr);
	gopidx = -1;
    CCpattern = 0;
    CCextra = 0;
    CCframes = 0;
    CCextraFields = 0;
	do {
		i = getbits(32);
nextpass:
		switch (i) {
			case GROUP_START_CODE:
                gopcaptions[gopidx++] = (CCpattern * 128) + (CCframes * 2) + CCextraFields;
                CCpattern = 0;
                CCextra = 0;
                CCframes = 0;
                CCextraFields = 0;
				break;
            case EXTENSION_START_CODE:
                j = getbits(4);
                if (j == PICTURE_CODING_EXTENSION) {
                  j = getbits(20); // unused
                  CCpattern = get1bit(); // Top_Field_First
                  j = getbits(5); // unused
                  CCextra = get1bit(); // Repeat_First_Field
                  if (CCextra == 1) {
                    CCextraFields++;
                  }
                  j = getbits(25); // unused
                }
                break;
			case PICTURE_START_CODE:
				CCframes++;
				break;
			case USER_DATA_START_CODE:
				cout << "Please remove all user data from input MPEG before running" << endl;
				exit(1);
			default:
				if ((i >> 8) != PACKET_START_CODE_PREFIX) {
					if (seek_sync(PACKET_START_CODE_PREFIX, 24)) {
						i = 0x00000100 | getbits(8);
						goto nextpass;
					}
					if (end_bs())
						break;
					cout << endl << endl << "Unknown bits in stream, could not resynch" << endl;
					exit(1);
				}
		}
	} while ((i != MPEG_PROGRAM_END_CODE) && (!end_bs()));

	finish_getbits();
    gopcaptions[gopidx++] = (CCpattern * 128) + (CCframes * 2) + CCextraFields;
	return;
}

void readGA(std::string GAinputfile) {
	char tmpStr[256];
	int i, j;
	size_t n;
	FILE *infp;
	
	using namespace std;
	n = GAinputfile.copy(tmpStr, 256, 0);
	tmpStr[n] = 0x00;
	infp = fopen(tmpStr, "r");
	if (!infp) {
		cout << "Unable to open " << GAinputfile << " for input" << endl;
		return;
	}
	if (fgets(tmpStr, 256, infp)) {
		gopidx = atoi(tmpStr);
	} else {
		fclose(infp);
		return;
	}
	for (i=0; i <= gopidx; i++) {
		if (fgets(tmpStr, 256, infp)) {
			gopcaptions[i] = atoi(tmpStr);
			// printf("%d.", gopcaptions[i]);
		} else {
			fclose(infp);
			return;
		}
	}
	fclose(infp);
}

void mux(std::string MPGinputfile, std::string BINinputfile, std::string MPGoutputfile, int useCCanalysis) {
	char tmpStr[256];
	unsigned long i, j, k;
	unsigned char CCpattern, CCextra, CCframes;
	size_t n;
	bool gop_started = false;

	using namespace std;
	n = MPGinputfile.copy(tmpStr, 256, 0);
	tmpStr[n] = 0x00;
	init_getbits(tmpStr);
	n = MPGoutputfile.copy(tmpStr, 256, 0);
	tmpStr[n] = 0x00;
	outfp = fopen(tmpStr, "wb");
	if (!outfp) {
		cout << "Unable to open " << MPGoutputfile << " for output" << endl;
		return;
	}
	n = BINinputfile.copy(tmpStr, 256, 0);
	tmpStr[n] = 0x00;
	binfp = fopen(tmpStr, "rb");
	if (!binfp) {
		cout << "Unable to open " << BINinputfile << " for input" << endl;
		return;
	}
	// skip first 4 bytes of BIN file
	n = fread(ccbuffer, sizeof(unsigned char), 4, binfp);
	if (n <= 0)
		fclose(binfp);

	do {
		i = getbits(32);
mainloop:
		switch (i) {
			case MPEG_PROGRAM_END_CODE:
				break;
			case GROUP_START_CODE:
				gop_started = true;
				gopidx++;
				break;
			case PICTURE_START_CODE:
				if (gop_started) {
					gop_started = false;
    				// printf("%d,%d ", gopidx, gopcaptions[gopidx]);
					if (gopcaptions[gopidx] > 0) {
						j = gopcaptions[gopidx];
						CCpattern = j >> 7;
						CCextra = j & 0x01;
						CCframes = j & 0x7f;
						CCframes >>= 1;
					} else {
						CCframes = 0;
					}
					if (CCframes == 0) {
						break;
					}
					// Output closed caption packet
					//  (located just before the first picture packet in each GOP)
					write_bytes(USER_DATA_START_CODE, 4);
					write_bytes(DVD_CLOSED_CAPTION, 4);
					// Attribute byte:
					//  2 x GOP size,
					//  plus 0 not to add an extra field,
					//  plus 0x80 to use the pattern Field 1, Field 2
					j = (CCframes*2) + CCextra + (CCpattern * 0x80);
					write1byte(j);
					if (CCextra)
						CCframes++;
					for (j=0; j<CCframes; j++) {
						// first field
						k = (CCpattern == 0) ? 0xFE : 0xFF;
						write1byte(k);
						if ((binfp) && (CCpattern == 1)) {
							n = fread(ccbuffer, sizeof(unsigned char), 2, binfp);
							if (n <= 0)
								fclose(binfp);
						}
						if (CCpattern == 1) {
							if (binfp) {
								// printf("%02x.%02x ", ccbuffer[0], ccbuffer[1]);
								write1byte(ccbuffer[0]);
								write1byte(ccbuffer[1]);
							} else {
								write_bytes(0x8080, 2);
							}
						} else {
							write_bytes(0x8080, 2);
						}
						if (((j+1) == CCframes) && CCextra) {
							continue;
						}
						// second field
						k = (CCpattern == 0) ? 0xFF : 0xFE;
						write1byte(k);
						if ((binfp) && (CCpattern == 0)) {
							n = fread(ccbuffer, sizeof(unsigned char), 2, binfp);
							if (n <= 0)
								fclose(binfp);
						}
						if (CCpattern == 0) {
							if (binfp) {
								write1byte(ccbuffer[0]);
								write1byte(ccbuffer[1]);
							} else {
								write_bytes(0x8080, 2);
							}
						} else {
							write_bytes(0x8080, 2);
						}
					}
				}
				break;
			default:
				if ((i >> 8) != PACKET_START_CODE_PREFIX) {
					// can't use seek_sync because we want to copy everything to outfp
					while ((i & 0xFFFFFF00) != PICTURE_START_CODE) {
						j = i >> 24;
						write1byte(j);
						i <<= 8;
						i &= 0xFFFFFFFF;
						i |= getbits(8);
						if (end_bs()) {
							break;
						}
					}
					if (end_bs()) {
						break;
					}
					goto mainloop;
				}
		}
		write_bytes(i, 4);
	} while ((i != MPEG_PROGRAM_END_CODE) && (!end_bs()));

	if (binfp) {
		fclose(binfp);
	}
	if (outfp) {
		flush_buffer();
		fclose(outfp);
	}
	finish_getbits();
	return;
}

void write_buffer()
{
  if (fwrite(buffer, sizeof(unsigned char), bufidx + 1, outfp) != (unsigned int)bufidx + 1)
  {
    printf("Error writing to output file.");
    exit (1);
  }
  bufidx = -1;
}

void flush_buffer()
{
  if (bufidx >= 0)
    write_buffer();
}

void write_bytes(unsigned int N, int size) {
	unsigned char i;
	int j;
	for (j=size-1; j>=0; j--) {
		i = (unsigned char)(N >> (j * 8));
		buffer[++bufidx] = i & 0xFF;
		if (bufidx == BUFFER_SIZE - 1)
			write_buffer();
	}
}

void write1byte(unsigned int N) {
	unsigned char i;
	i = (unsigned char)N;
	buffer[++bufidx] = i;
	if (bufidx == BUFFER_SIZE - 1) {
		write_buffer();
	}
}

